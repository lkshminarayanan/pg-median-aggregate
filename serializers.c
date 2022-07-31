#include "serializers.h"

#include <catalog/pg_type.h>

/*
 * serialize_* functions
 *	Oid specific serializer methods
 */

void
serialize_int32(StringInfo buf, Datum datum)
{
	pq_sendint32(buf, DatumGetInt32(datum));
}

void
serialize_int64(StringInfo buf, Datum datum)
{
	pq_sendint64(buf, DatumGetInt64(datum));
}

void
serialize_float4(StringInfo buf, Datum datum)
{
	pq_sendfloat4(buf, DatumGetFloat4(datum));
}

void
serialize_text(StringInfo buf, Datum datum)
{
	text	   *tPtr = DatumGetTextPP(datum);
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

Datum
deserialize_int32(StringInfo buf, MemoryContext agg_context)
{
	return Int32GetDatum(pq_getmsgint(buf, 4));
}

Datum
deserialize_int64(StringInfo buf, MemoryContext agg_context)
{
	return Int64GetDatum(pq_getmsgint64(buf));
}

Datum
deserialize_float4(StringInfo buf, MemoryContext agg_context)
{
	return Float4GetDatum(pq_getmsgfloat4(buf));
}

Datum
deserialize_text(StringInfo buf, MemoryContext agg_context)
{
	int32		textLen = pq_getmsgint(buf, 4);
	const char *data = pq_getmsgbytes(buf, textLen);
	int32		varsize = textLen + VARHDRSZ;
	text	   *result = (text *) MemoryContextAlloc(agg_context,
													 varsize);

	SET_VARSIZE(result, varsize);
	memcpy(VARDATA(result), data, textLen);

	PG_RETURN_TEXT_P(result);
}

deserializer
get_deserializer(Oid datum_type)
{
	switch (datum_type)
	{
		case INT2OID:
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
