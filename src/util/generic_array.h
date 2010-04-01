/* Copyright 2003 Jacob Robbins*/
/*released under GPL*/



#ifndef UTIL_GENERIC_ARRAY_INCLUDES
#define UTIL_GENERIC_ARRAY_INCLUDES

typedef struct generic_array generic_array_t;

generic_array_t* generic_array_create(size_t element_size);
void generic_array_destroy(generic_array_t* array);
generic_array_t* generic_array_copy(const generic_array_t* array);

size_t generic_array_get_element_size(const generic_array_t* array);
int generic_array_get_size(const generic_array_t* array);
int generic_array_set_size(generic_array_t* array, int newsize);
int generic_array_get_element(const generic_array_t* array, int pos, void* target);
int generic_array_set_element(generic_array_t* array, int pos, void* target);
int generic_array_insert_element(generic_array_t* array, int pos, void* target);
int generic_array_remove_element(generic_array_t* array, int pos, void* target);
int generic_array_append_element(generic_array_t* array, void* target);
void* generic_array_get_data_pointer(const generic_array_t* array);
void* generic_array_get_element_pointer(const generic_array_t* array, int pos);


#endif
