/* Copyright 2003 Jacob Robbins*/
/*released under GPL*/

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "../include.h"


struct generic_array{
  size_t element_size;
  int arr_size;
  void * arr;
};

generic_array_t* generic_array_create(size_t element_size){
  generic_array_t* new_arr;
  if (!(new_arr = (generic_array_t*)malloc(sizeof(generic_array_t)))) return 0;
  new_arr->element_size = element_size;
  new_arr->arr_size = 0;
  new_arr->arr = 0;
  return new_arr;
}

void generic_array_destroy(generic_array_t* array){
  if (array->arr) free(array->arr);
  free(array);
}

generic_array_t* generic_array_copy(const generic_array_t* array){
  int i;
  generic_array_t* new_arr;
  void* curr;

  if (!(new_arr = generic_array_create(generic_array_get_element_size(array)))){
    return 0;
  }

  for (i=0;i<generic_array_get_size(array);++i){

    curr = generic_array_get_element_pointer(array,i);
    generic_array_append_element(new_arr,curr);

  }

  return new_arr;
}

size_t generic_array_get_element_size(const generic_array_t* array){
  return array->element_size;
}

int generic_array_get_size(const generic_array_t* array){
  return array->arr_size;
}

int generic_array_set_size(generic_array_t* array, int newsize){
  void * new_arr;

  if (!(new_arr = (void*)malloc(newsize * array->element_size))) return -1;

  if (newsize < array->arr_size){
    memcpy(new_arr, array->arr,newsize*array->element_size);
  }else{
    memcpy(new_arr, array->arr,array->arr_size*array->element_size);
    memset(new_arr + (array->arr_size * array->element_size), 0, (newsize - array->arr_size)*array->element_size);
  }
    
  free(array->arr);
  array->arr = new_arr;
  array->arr_size = newsize;
  return newsize;
}


int generic_array_get_element(const generic_array_t* array, int pos, void* target){
  if ((pos >= array->arr_size)||(pos < 0)) return -1;
  memcpy(target,array->arr + (pos * array->element_size),  array->element_size);
  return 0;
}

int generic_array_set_element(generic_array_t* array, int pos, void* target){
  if ((pos >= array->arr_size)||(pos < 0)) return -1;
  if (target){
    memcpy(array->arr + (pos * array->element_size), target, array->element_size);
  }else{
    memset(array->arr + (pos * array->element_size), 0, array->element_size);
  }
  return 0;
}


int generic_array_insert_element(generic_array_t* array, int pos, void* target){
  int i;
  if ((pos > array->arr_size)||(pos < 0)) return -1;
  if (pos == array->arr_size) return generic_array_append_element(array, target);

  if ((generic_array_set_size(array, array->arr_size + 1))<0) return -1;

  if (array->arr_size > 1){
    for (i=array->arr_size - 1; i > pos; --i){
      memcpy(array->arr + (i * array->element_size), array->arr + ((i-1) * array->element_size), array->element_size);
    }
  }

  if (target){
    memcpy(array->arr + (pos * array->element_size), target, array->element_size);
  }else{
    memset(array->arr + (pos * array->element_size), 0, array->element_size);
  }

  return 0;
}


int generic_array_remove_element(generic_array_t* array, int pos, void* target){
  int i;
  if (array->arr_size <= 0) return -1;
  if ((pos >= array->arr_size)||(pos < 0)) return -1;

  if (target){
      memcpy(target, array->arr + (pos * array->element_size), array->element_size);
  }

  if (array->arr_size > 1){
    for (i=pos; i < array->arr_size - 1; ++i){
      memcpy(array->arr + (i * array->element_size), array->arr + ((i+1) * array->element_size), array->element_size);
    }
  }

  if ((generic_array_set_size(array, array->arr_size - 1))<0) return -1;

  return 0;
}

int generic_array_append_element(generic_array_t* array, void* target){
  if ((generic_array_set_size(array, array->arr_size + 1))<0) return -1;

  if (target)
    memcpy(array->arr + ((array->arr_size - 1) * array->element_size), target, array->element_size);
  else
    memset(array->arr + ((array->arr_size - 1) * array->element_size), 0, array->element_size);
   
  return 0;
}

void* generic_array_get_data_pointer(const generic_array_t* array) {return array->arr;}

void* generic_array_get_element_pointer(const generic_array_t* array, int pos) {

  if ((pos > array->arr_size)||(pos < 0)) return 0;  
  return array->arr + (pos * array->element_size);

}
