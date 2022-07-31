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
 * array_create_with_capacity
 *	 Create a new Array struct variable with specified capacity in
 *	 the given MemoryContext
 */
Array *
array_create_with_capacity(MemoryContext agg_context, int32 capacity, Oid type)
{
	Array	   *array = (Array *) MemoryContextAllocZero(agg_context,
														 sizeof(Array));

	array->capacity = capacity;
	array->length = 0;
	array->type = type;
	array->data = _alloc_array_data(agg_context, capacity);
	return array;
}

/*
 * array_create
 *	 Create a new Array struct variable in given MemoryContext
 */
Array *
array_create(MemoryContext agg_context, Oid type)
{
	return array_create_with_capacity(agg_context, 1, type);
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
 * _array_extend_capacity
 *	 Extend the capacity by allocating a new data array and copying the
 *	 existing elements into there.
 */
void
_array_extend_capacity(Array * array, MemoryContext agg_context,
					   int32 new_capacity)
{
	Datum	   *new_data = _alloc_array_data(agg_context,
											 new_capacity);

	/* copy the elements into the new memory and update array members */
	memmove(new_data, array->data, sizeof(Datum) * array->capacity);
	pfree(array->data);
	array->data = new_data;
	array->capacity = new_capacity;
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
		_array_extend_capacity(array, agg_context, array->capacity * 2);
	}

	/* insert data at the end */
	array->data[array->length++] = value;
}

/*
 * array_combine
 *	 Merges the given source array into dest array and
 *	 returns a pointer to dest Array.
 */
Array *
array_combine(MemoryContext agg_context, Array * dest, const Array * source)
{
	if (dest == NULL)
	{
		/* create a new Array as dest is NULL */
		dest = array_create_with_capacity(agg_context, source->length,
										  source->type);
	}
	else if (dest->capacity - dest->length < source->length)
	{
		/* extend capacity of dest */
		_array_extend_capacity(dest, agg_context,
							   dest->length + source->length);
	}

	/* Copy the source elements into dest */
	Datum	   *dest_offset = &(dest->data[dest->length]);

	memcpy(dest_offset, source->data, source->length * sizeof(Datum));
	dest->length += source->length;

	return dest;
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
