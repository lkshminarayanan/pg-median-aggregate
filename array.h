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
Array	   *array_create(MemoryContext agg_context, Oid type);
void		array_insert(Array * array, MemoryContext agg_context, Datum value);
void		array_qsort(Array * array);
void		array_free(Array * array);

#endif   /* STATE_ARRAY_H */
