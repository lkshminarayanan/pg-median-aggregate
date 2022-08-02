#include "array.h"

#include <catalog/pg_type.h>
#include "comparators.h"

#define AVG(a,b) ((a)/2 + (b)/2)

/*
 * _alloc_array_data
 *	 Create a new Datum array with given capacity
 *
 * Internal function to be used by the array functions.
 */
Pointer
_alloc_array_data(MemoryContext agg_context, int32 capacity, int32 element_size)
{
	Pointer		data = (Pointer) MemoryContextAllocZero(agg_context,
													element_size * capacity);

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
array_create_with_capacity(MemoryContext agg_context, Oid type, int32 capacity)
{
	Array	   *array = (Array *) MemoryContextAllocZero(agg_context,
														 sizeof(Array));

	array->capacity = capacity;
	array->length = 0;
	array->type = type;
	array->array_setter = get_pointer_array_setter_and_element_size(
												 type, &array->element_size);
	array->data = _alloc_array_data(
								 agg_context, capacity, array->element_size);
	return array;
}

/*
 * array_create
 *	 Create a new Array struct variable in given MemoryContext
 */
Array *
array_create(MemoryContext agg_context, Oid type)
{
	return array_create_with_capacity(agg_context, type, 1);
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
	Pointer		new_data = _alloc_array_data(agg_context,
											 new_capacity,
											 array->element_size);

	/* copy the elements into the new memory and update array members */
	memmove(new_data, array->data, array->element_size * array->capacity);
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
	array->array_setter(array->data, value, array->length++);
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
		dest = array_create_with_capacity(agg_context, source->type,
										  source->length);
	}
	else if (dest->capacity - dest->length < source->length)
	{
		/* extend capacity of dest */
		_array_extend_capacity(dest, agg_context,
							   dest->length + source->length);
	}

	/* Copy the source elements into dest */
	Pointer		dest_offset = &(dest->data[dest->length * dest->element_size]);

	memcpy(dest_offset, source->data, source->length * source->element_size);
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
		qsort(array->data, array->length, array->element_size,
			  get_comparator(array->type));
	}
	else
	{
		/* collatable data type */
		qsort_arg(array->data, array->length, array->element_size,
				  get_comparator(array->type), &collation_oid);
	}
}

Datum
array_get_median(Array * array, Oid collation_oid)
{
	/* Sort the values in the array */
	array_qsort(array, collation_oid);

	/* calculate and return the median value */
	Pointer		median1 = &array->data[(array->length / 2) * array->element_size];

	if (array->length % 2 == 1)
	{
		/* Even array length */
		switch (array->type)
		{
			case INT2OID:
				return Int16GetDatum(*(int16 *) median1);
			case INT4OID:
				return Int32GetDatum(*(int32 *) median1);
			case TIMESTAMPTZOID:
				return Int64GetDatum(*(int64 *) median1);
			case FLOAT4OID:
				return Float4GetDatum(*(float4 *) median1);
			case TEXTOID:
				return PointerGetDatum(*(text **) median1);
			default:
				elog(ERROR, "Unsupported data type");
		}
	}

	/*
	 * even array length pickup the (n/2)-1 element for median calculation
	 */
	Pointer		median2 = median1 - array->element_size;

	/* calculate average of median1 and median2 as the mean */
	switch (array->type)
	{
		case INT2OID:
			return Int16GetDatum(AVG(*(int16 *) median1, *(int16 *) median2));
		case INT4OID:
			return Int32GetDatum(AVG(*(int32 *) median1, *(int32 *) median2));
		case TIMESTAMPTZOID:
			return Int64GetDatum(AVG(*(int64 *) median1, *(int64 *) median2));
		case FLOAT4OID:
			return Float4GetDatum(AVG(*(float4 *) median1, *(float4 *) median2));
		case TEXTOID:
			elog(WARNING, "Cannot calculate mean for text array with even length");
			return PointerGetDatum(NULL);
		default:
			elog(ERROR, "Unsupported data type");
	}
}
