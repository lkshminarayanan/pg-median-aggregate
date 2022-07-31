#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/varlena.h>

#include "comparators.h"

/* Comparator functions for various postgres Oid types
 *	 All comparators return
 *		- negative value if l < r
 *		- 0 if l == r and
 *		- positive value if l > r
 */

#define COMPARE(a,b) (((a) > (b)) - ((a) < (b)))

/*
 * datum_comparator_int64
 *	 Comparator for all numeric data types that can fit into 64 bytes
 */
int
datum_comparator_int64(const void *l, const void *r)
{
	return COMPARE(DatumGetInt64(*(const Datum *) l),
				   DatumGetInt64(*(const Datum *) r));
}

/*
 * datum_comparator_float4
 *	 Comparator for float4 data type
 */
int
datum_comparator_float4(const void *l, const void *r)
{
	return COMPARE(DatumGetFloat4(*(const Datum *) l),
				   DatumGetFloat4(*(const Datum *) r));
}

/*
 * datum_comparator_text
 *	 Comparator for text data type
 */
int
datum_comparator_text(const void *l, const void *r, void *arg)
{
	const text *lValue = DatumGetTextP(*(const Datum *) l);
	const text *rValue = DatumGetTextP(*(const Datum *) r);

	if (arg == NULL)
	{
		elog(ERROR,
			 "NULL collation oid sent to datum_comparator_text function");
	}

	return varstr_cmp(VARDATA(lValue), VARSIZE(lValue),
					  VARDATA(rValue), VARSIZE(rValue),
					  *(Oid *) arg);
}

/*
 * get_comparator
 *	 Returns the matching comparator for the given type
 */
void *
get_comparator(Oid datum_type)
{
	switch (datum_type)
	{
		case INT2OID:
		case INT4OID:
		case TIMESTAMPTZOID:	/* TimestampTz is internally a int64 type */
			return datum_comparator_int64;
		case FLOAT4OID:
			return datum_comparator_float4;
		case TEXTOID:
			return datum_comparator_text;
		case InvalidOid:
			elog(ERROR, "invalid oid type");
		default:
			elog(ERROR, "unsupported data type");
	}
}
