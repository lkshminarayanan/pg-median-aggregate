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
 * datum_comparator_int16
 *	 Comparator for int16 datatype
 */
int
datum_comparator_int16(const void *l, const void *r)
{
	return COMPARE(*(int16 *) l, *(int16 *) r);
}

/*
 * datum_comparator_int32
 *	 Comparator for int32 datatype
 */
int
datum_comparator_int32(const void *l, const void *r)
{
	return COMPARE(*(int32 *) l, *(int32 *) r);
}

/*
 * datum_comparator_int64
 *	 Comparator for all numeric data types that can fit into 64 bytes
 */
int
datum_comparator_int64(const void *l, const void *r)
{
	return COMPARE(*(int64 *) l, *(int64 *) r);
}

/*
 * datum_comparator_float4
 *	 Comparator for float4 data type
 */
int
datum_comparator_float4(const void *l, const void *r)
{
	return COMPARE(*(float4 *) l, *(float4 *) r);
}

/*
 * datum_comparator_text
 *	 Comparator for text data type
 */
int
datum_comparator_text(const void *l, const void *r, void *arg)
{
	const text *lValue = *(text **) l;
	const text *rValue = *(text **) r;

	if (arg == NULL || *(Oid *) arg == InvalidOid)
	{
		elog(ERROR,
			 "NULL or Invalid collation oid sent to datum_comparator_text function");
	}

	return varstr_cmp(VARDATA_ANY(lValue), VARSIZE_ANY_EXHDR(lValue),
					  VARDATA_ANY(rValue), VARSIZE_ANY_EXHDR(rValue),
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
			return datum_comparator_int16;
		case INT4OID:
			return datum_comparator_int32;
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
