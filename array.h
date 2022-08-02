#ifndef STATE_ARRAY_H
#define STATE_ARRAY_H

#include <postgres.h>
#include "pointer_array_utils.h"

/* Array struct maintains the state of the aggregate */
typedef struct Array
{
	int32		capacity;		/* total capacity of the array */
	int32		length;			/* number of elements in the array */
	Oid			type;			/* Oid type of the element */
	Pointer		data;			/* Actual data array */
	int32		element_size;	/* byte size of a single element */
	pointer_array_setter_func array_setter;		/* insert func to add elements
												 * to the array */
}	Array;

/* Array functions */
Array	   *array_create_with_capacity(MemoryContext agg_context, Oid type, int32 capacity);
Array	   *array_create(MemoryContext agg_context, Oid type);
void		array_insert(Array * array, MemoryContext agg_context, Datum value);
Array	   *array_combine(MemoryContext agg_context, Array * dest, const Array * source);
void		array_qsort(Array * array, Oid collation_oid);
void		array_free(Array * array);
Datum		array_get_median(Array * array, Oid collation_oid);

#endif   /* STATE_ARRAY_H */
