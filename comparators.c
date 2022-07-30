#include <postgres.h>
#include <catalog/pg_type.h>

#include "comparators.h"

/* Comparator functions for various postgres Oid types
 *	 All comparators return
 *		- negative value if l < r
 *		- 0 if l == r and
 *		- positive value if l > r
 */

/*
 * datum_comparator_integer
 *	 Comparator for integer data types
 */
int
datum_comparator_integer(const void *l, const void *r)
{
	const Datum lValue = *(const Datum *) l;
	const Datum rValue = *(const Datum *) r;

	return lValue - rValue;
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
			return datum_comparator_integer;
		case InvalidOid:
			elog(ERROR, "invalid oid type");
		default:
			elog(ERROR, "unsupported data type");
	}
}
