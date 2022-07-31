#include "array.h"
#include "comparators.h"

/*
 * _alloc_array_data
 *	 Create a new Datum array with given capacity
 *
 * Internal function to be used by the array functions.
 */
Datum *
_alloc_array_data(MemoryContext agg_context, int32 capacity)
{
	Datum	   *data = (Datum *) MemoryContextAllocZero(agg_context, sizeof(Datum) * capacity);

	if (data == NULL)
	{
		elog(ERROR, "alloc_array_data ran out of memory");
	}
	return data;
}

/*
 * array_create
 *	 Create a new Array struct variable in given MemoryContext
 */
Array *
array_create(MemoryContext agg_context, Oid type)
{
	Array	   *array = (Array *) MemoryContextAllocZero(agg_context,
														 sizeof(Array));

	array->capacity = 1;
	array->length = 0;
	array->type = type;
	array->data = _alloc_array_data(agg_context, array->capacity);
	return array;
}

/*
 * array_free
 *	 Free the memory allocated for the Array
 */
void
array_free(Array * array)
{
	pfree(array->data);
	pfree(array);
}

/*
 * array_insert
 *	 Inserts the given value in Array.
 *
 * If the capacity is not sufficient, a new data with double the current
 * capacity will be allocated and the existing elements will be copied there.
 */
void
array_insert(Array * array, MemoryContext agg_context, Datum value)
{
	if (array->length == array->capacity)
	{
		/* array has run out of capacity - extend it */
		int32		current_capacity = array->capacity;

		array->capacity *= 2;
		Datum	   *new_data = _alloc_array_data(agg_context,
												 array->capacity);

		/* copy the elements into the new memory and update array members */
		memmove(new_data, array->data, sizeof(Datum) * current_capacity);
		pfree(array->data);
		array->data = new_data;
	}

	/* insert data at the end */
	array->data[array->length++] = value;
}

/*
 * array_qsort
 *	 Sorts the values stored in the array
 */
void
array_qsort(Array * array, Oid collation_oid)
{
	if (collation_oid == InvalidOid)
	{
		/* Array has a non collatable data type */
		qsort(array->data, array->length, sizeof(Datum),
			  get_comparator(array->type));
	}
	else
	{
		/* collatable data type */
		qsort_arg(array->data, array->length, sizeof(Datum),
				  get_comparator(array->type), &collation_oid);
	}
}
