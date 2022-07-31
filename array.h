#ifndef STATE_ARRAY_H
#define STATE_ARRAY_H

#include <postgres.h>

/* Array struct maintains the state of the aggregate */
typedef struct Array
{
	int32		capacity;
	int32		length;
	Oid			type;
	Datum	   *data;
}	Array;

/* Array functions */
Array	   *array_create_with_capacity(MemoryContext agg_context, int32 capacity, Oid type);
Array	   *array_create(MemoryContext agg_context, Oid type);
void		array_insert(Array * array, MemoryContext agg_context, Datum value);
Array	   *array_combine(MemoryContext agg_context, Array * dest, const Array * source);
void		array_qsort(Array * array, Oid collation_oid);
void		array_free(Array * array);

#endif   /* STATE_ARRAY_H */
