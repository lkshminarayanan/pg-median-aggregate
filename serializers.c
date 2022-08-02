#include "serializers.h"

#include <catalog/pg_type.h>

/*
 * serialize_* functions
 *	Oid specific serializer methods
 */

void
serialize_int16(StringInfo buf, const Pointer p)
{
	pq_sendint16(buf, *(int16 *) p);
}

void
serialize_int32(StringInfo buf, const Pointer p)
{
	pq_sendint32(buf, *(int32 *) p);
}

void
serialize_int64(StringInfo buf, const Pointer p)
{
	pq_sendint64(buf, *(int64 *) p);
}

void
serialize_float4(StringInfo buf, const Pointer p)
{
	pq_sendfloat4(buf, *(float4 *) p);
}

void
serialize_text(StringInfo buf, const Pointer p)
{
	text	   *tPtr = *(text **) p;
	int32		textLen = VARSIZE_ANY_EXHDR(tPtr);

	pq_sendint32(buf, textLen);
	pq_sendbytes(buf, VARDATA_ANY(tPtr), textLen);
}

serializer
get_serializer(Oid datum_type)
{
	switch (datum_type)
	{
		case INT2OID:
			return serialize_int16;
		case INT4OID:
			return serialize_int32;
		case FLOAT4OID:
			return serialize_float4;
		case TEXTOID:
			return serialize_text;
		case TIMESTAMPTZOID:	/* TimestampTz is internally a int64 type */
			return serialize_int64;
		case InvalidOid:
			elog(ERROR, "invalid oid type");
		default:
			elog(ERROR, "unsupported data type");
	}
}

/*
 * deserialize_* functions
 *	Oid specific serializer methods
 */

void
deserialize_int16(StringInfo buf, MemoryContext agg_context, Pointer p)
{
	*(int16 *) p = pq_getmsgint(buf, 2);
}

void
deserialize_int32(StringInfo buf, MemoryContext agg_context, Pointer p)
{
	*(int32 *) p = pq_getmsgint(buf, 4);
}

void
deserialize_int64(StringInfo buf, MemoryContext agg_context, Pointer p)
{
	*(int64 *) p = pq_getmsgint64(buf);
}

void
deserialize_float4(StringInfo buf, MemoryContext agg_context, Pointer p)
{
	*(float4 *) p = pq_getmsgfloat4(buf);
}

void
deserialize_text(StringInfo buf, MemoryContext agg_context, Pointer p)
{
	int32		textLen = pq_getmsgint(buf, 4);
	const char *data = pq_getmsgbytes(buf, textLen);
	int32		varsize = textLen + VARHDRSZ;
	text	   *result = (text *) MemoryContextAlloc(agg_context,
													 varsize);

	SET_VARSIZE(result, varsize);
	memcpy(VARDATA(result), data, textLen);

	*(text **) p = result;
}

deserializer
get_deserializer(Oid datum_type)
{
	switch (datum_type)
	{
		case INT2OID:
			return deserialize_int16;
		case INT4OID:
			return deserialize_int32;
		case FLOAT4OID:
			return deserialize_float4;
		case TEXTOID:
			return deserialize_text;
		case TIMESTAMPTZOID:	/* TimestampTz is internally a int64 type */
			return deserialize_int64;
		case InvalidOid:
			elog(ERROR, "invalid oid type");
		default:
			elog(ERROR, "unsupported data type");
	}
}
