#include "pointer_array_utils.h"

#include <postgres.h>
#include <catalog/pg_type.h>

/* Oid specific pointer array setters */
void
pointer_array_set_int16(Pointer data, Datum d, int32 idx)
{
	((int16 *) data)[idx] = DatumGetInt16(d);
}

void
pointer_array_set_int32(Pointer data, Datum d, int32 idx)
{
	((int32 *) data)[idx] = DatumGetInt32(d);
}

void
pointer_array_set_int64(Pointer data, Datum d, int32 idx)
{
	((int64 *) data)[idx] = DatumGetInt64(d);
}

void
pointer_array_set_float4(Pointer data, Datum d, int32 idx)
{
	((float4 *) data)[idx] = DatumGetFloat4(d);
}

void
pointer_array_set_text(Pointer data, Datum d, int32 idx)
{
	((text **) data)[idx] = DatumGetTextPP(d);
}

/*
 * get_pointer_array_setter_and_element_size
 *	 Returns the setter to be used to set a datum element to the
 *	 pointer array and the element size based on the Oid data type.
 */
pointer_array_setter_func
get_pointer_array_setter_and_element_size(Oid type, int *element_size)
{
	switch (type)
	{
		case INT2OID:
			*element_size = sizeof(int16);
			return pointer_array_set_int16;
		case INT4OID:
			*element_size = sizeof(int32);
			return pointer_array_set_int32;
		case TIMESTAMPTZOID:	/* TimestampTz is internally a int64 type */
			*element_size = sizeof(int64);
			return pointer_array_set_int64;
		case FLOAT4OID:
			*element_size = sizeof(float4);
			return pointer_array_set_float4;
		case TEXTOID:
			*element_size = sizeof(text *);
			return pointer_array_set_text;
		case InvalidOid:
			elog(ERROR, "invalid oid type");
		default:
			elog(ERROR, "unsupported data type");
	}
}
