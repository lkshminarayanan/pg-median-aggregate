#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
#include <libpq/pqformat.h>
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

	/* Extract the median */
	Datum		median = array_get_median(state, PG_GET_COLLATION());

	array_free(state);

	return median;
}

PG_FUNCTION_INFO_V1(median_combinefn);

/*
 * Median state combine function.
 *
 * This function is called to combine two partial aggregate states.
 */
Datum
median_combinefn(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_combinefn called in non-aggregate context");

	Array	   *state1 = (Array *) (PG_ARGISNULL(0) ? NULL : PG_GETARG_POINTER(0));
	Array	   *state2 = (Array *) (PG_ARGISNULL(1) ? NULL : PG_GETARG_POINTER(1));

	Array	   *combined_state = NULL;

	if (state1 != NULL)
	{
		combined_state = array_combine(agg_context, combined_state, state1);
	}

	if (state2 != NULL)
	{
		combined_state = array_combine(agg_context, combined_state, state2);
	}

	PG_RETURN_POINTER(combined_state);
}

PG_FUNCTION_INFO_V1(median_serializefn);

/* Median state serializer */
Datum
median_serializefn(PG_FUNCTION_ARGS)
{
	Assert(!PG_ARGISNULL(0));
	Array	   *state = (Array *) PG_GETARG_POINTER(0);

	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendint32(&buf, state->length);
	pq_sendint32(&buf, state->type);

	if (state->type == TEXTOID)
	{
		/* text type has to be serialized per element */
		int32		textLen;
		text	  **tPtr = (text **) state->data;

		for (int i = 0; i < state->length; i++, tPtr++)
		{
			textLen = VARSIZE_ANY_EXHDR(*tPtr);

			pq_sendint32(&buf, textLen);
			pq_sendbytes(&buf, VARDATA_ANY(*tPtr), textLen);
		}
	}
	else
	{
		/* native types can be copied directly from array */
		pq_sendbytes(&buf, state->data, state->length * state->element_size);
	}

	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(median_deserializefn);

/* Median state deserializer */
Datum
median_deserializefn(PG_FUNCTION_ARGS)
{
	MemoryContext agg_context;

	if (!AggCheckCallContext(fcinfo, &agg_context))
		elog(ERROR, "median_deserializefn called in non-aggregate context");

	Assert(!PG_ARGISNULL(0));
	bytea	   *serialized = PG_GETARG_BYTEA_P(0);

	StringInfoData buf;

	buf.data = VARDATA(serialized);
	buf.len = VARSIZE(serialized) - VARHDRSZ;
	buf.maxlen = VARSIZE(serialized) - VARHDRSZ;
	buf.cursor = 0;

	int32		array_length = pq_getmsgint(&buf, 4);
	Oid			type = pq_getmsgint(&buf, 4);
	Array	   *state = array_create_with_capacity(agg_context, type, array_length);

	if (state->type == TEXTOID)
	{
		/* text type has to be deserialized per element */
		int32		textLen;
		const char *data;
		int32		varsize;
		text	   *result;
		text	  **tPtr = (text **) state->data;

		for (int i = 0; i < array_length; i++, tPtr++)
		{
			textLen = pq_getmsgint(&buf, 4);
			data = pq_getmsgbytes(&buf, textLen);
			varsize = textLen + VARHDRSZ;
			result = (text *) MemoryContextAlloc(agg_context,
												 varsize);
			SET_VARSIZE(result, varsize);
			memcpy(VARDATA(result), data, textLen);
			*tPtr = result;
		}
	}
	else
	{
		/* native types can be copied directly into array */
		int			array_size_in_bytes = array_length * state->element_size;
		const char *data = pq_getmsgbytes(&buf, array_size_in_bytes);

		memcpy(state->data, data, array_size_in_bytes);
	}

	state->length = array_length;
	pq_getmsgend(&buf);

	PG_RETURN_POINTER(state);
}
