#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
#include "array.h"

#if PG_VERSION_NUM < 120000 || PG_VERSION_NUM >= 130000
#error "Unsupported PostgreSQL version. Use version 12."
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(median_transfn);

/*
 * Median state transfer function.
 *
 * This function is called for every value in the set that we are calculating
 * the median for. On first call, the aggregate state, if any, needs to be
 * initialized.
 */
Datum
median_transfn(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_transfn called in non-aggregate context");

	/* Retrieve the old state */
	Array	   *state = (Array *) (PG_ARGISNULL(0) ? NULL : PG_GETARG_POINTER(0));

	if (!PG_ARGISNULL(1))
	{
		/* value not null - append it inside the state array */
		if (state == NULL)
		{
			/* create a new state */
			Oid			type = get_fn_expr_argtype(fcinfo->flinfo, 1);

			state = array_create(agg_context, type);
		}
		array_insert(state, agg_context, PG_GETARG_DATUM(1));
	}

	PG_RETURN_POINTER(state);
}

/* Calculate the average of 2 given elements */
Datum
calculate_avg(Datum a, Datum b, Oid datum_type)
{
	switch (datum_type)
	{
		case INT2OID:
		case INT4OID:
		case TIMESTAMPTZOID:
			{
				int64		af = DatumGetInt64(a);
				int64		bf = DatumGetInt64(b);

				/* TODO: Return a mean with decimal value intact */
				return Int64GetDatum((float) (af + bf) / 2);
			}
		case FLOAT4OID:
			{
				float4		af = DatumGetFloat4(a);
				float4		bf = DatumGetFloat4(b);

				return Float4GetDatum(af / 2 + bf / 2);
			}
		case TEXTOID:
			elog(WARNING, "Cannot calculate average for 2 text values");
			return PointerGetDatum(NULL);
		default:
			elog(WARNING, "Unsupported data type");
			return PointerGetDatum(NULL);
	}
}

PG_FUNCTION_INFO_V1(median_finalfn);

/*
 * Median final function.
 *
 * This function is called after all values in the median set has been
 * processed by the state transfer function. It should perform any necessary
 * post processing and clean up any temporary state.
 */
Datum
median_finalfn(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_finalfn called in non-aggregate context");

	Array	   *state = (Array *) (PG_ARGISNULL(0) ? NULL : PG_GETARG_POINTER(0));

	if (state == NULL)
	{
		/* query either returned 0 rows or only rows with NULL value */
		PG_RETURN_NULL();
	}

	/* Sort the values in the array */
	array_qsort(state, PG_GET_COLLATION());

	/* calculate and return the median value */
	Datum		median = state->data[state->length / 2];

	if (state->length % 2 == 0)
	{
		/* even array length */
		median = calculate_avg(median, state->data[state->length / 2 - 1],
							   state->type);
	}

	array_free(state);
	return median;
}
