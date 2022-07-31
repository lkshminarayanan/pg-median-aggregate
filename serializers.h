#ifndef DATUM_SERIALIZERS_H
#define DATUM_SERIALIZERS_H

#include <postgres.h>
#include <libpq/pqformat.h>

/* serializer function type */
typedef void (*serializer) (StringInfo buf, Datum datum);

/* Returns the matching serializer for the datum type */
serializer	get_serializer(Oid datum_type);

/* deserializer function type */
typedef Datum (*deserializer) (StringInfo buf, MemoryContext agg_context);

/* Returns the matching deserializer for the datum type */
deserializer get_deserializer(Oid datum_type);

#endif   /* DATUM_SERIALIZERS_H */