#ifndef POINTER_UTILS_H
#define POINTER_UTILS_H

#include <postgres.h>

/* function type for pointer array setters */
typedef void (*pointer_array_setter_func) (Pointer data_arr, Datum d, int32 index);

pointer_array_setter_func get_pointer_array_setter_and_element_size(Oid type, int *element_size);

#endif   /* POINTER_UTILS_H */
