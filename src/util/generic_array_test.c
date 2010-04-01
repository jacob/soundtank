/*Copyright 2003 Jacob Robbins*/

#include <stdio.h>

#include "generic_array.h"

struct my_midi_note {int abyte; int bbyte;};
typedef struct my_midi_note my_midi_note_t;
generic_array_t * local_arr_1;
generic_array_t * local_arr_2;

void print_my_midi_note(my_midi_note_t* note){
  printf("%d,%d ",note->abyte,note->bbyte);
}

void print_local_arr(){
  int i;
  my_midi_note_t note;
  my_midi_note_t* ret;
  ret = &note;

  printf("\nlocal array 1: size = %d elements, element size = %d bytes\n", \
	 generic_array_get_size(local_arr_1), generic_array_get_element_size(local_arr_1));

  for (i=0;i<generic_array_get_size(local_arr_1);++i){
    generic_array_get_element(local_arr_1,i,ret);
    printf("note %d: ",i);
    print_my_midi_note(ret);
  }

}





int main(int argc, char** argv){
  int i;
  my_midi_note_t local_note_1;

  local_arr_1 = generic_array_create(sizeof(my_midi_note_t));

  printf("\n\ncreated array with element size %d bytes\n",generic_array_get_element_size(local_arr_1));

  printf("\nsetting size to 10 elements\n");
  if ((generic_array_set_size(local_arr_1,10))<0) {printf("error setting size");}
  printf("\nsize is now %d elements\n",generic_array_get_size(local_arr_1));

  for (i=0;i<10;++i){
    local_note_1.abyte = 11 + i;  
    local_note_1.bbyte = 15 + i; 
    generic_array_set_element(local_arr_1,i,&local_note_1);
  }
  print_local_arr();

  printf("\n\n\nsetting element at position 0 to (33,33) \n");
  local_note_1.abyte = 33;  local_note_1.bbyte = 33; 
  generic_array_set_element(local_arr_1,0,&local_note_1);
  print_local_arr();

  printf("\n\n\nsetting element at position 5 to (55,55) \n");
  local_note_1.abyte = 55;  local_note_1.bbyte = 55; 
  generic_array_set_element(local_arr_1,5,&local_note_1);
  print_local_arr();

  printf("\n\n\nsetting element at position 9 to (99,99) \n");
  local_note_1.abyte = 99;  local_note_1.bbyte = 99; 
  generic_array_set_element(local_arr_1,9,&local_note_1);
  print_local_arr();

  printf("\n\n\ninserting element (-1,-1) at position 0 \n");
  local_note_1.abyte = -1;  local_note_1.bbyte = -1; 
  generic_array_insert_element(local_arr_1,0,&local_note_1);
  print_local_arr();

  printf("\n\n\ninserting element (-2,-2) at position 1 \n");
  local_note_1.abyte = -2;  local_note_1.bbyte = -2; 
  generic_array_insert_element(local_arr_1,1,&local_note_1);
  print_local_arr();

  printf("\n\n\ninserting element (-5,-5) at position 5 \n");
  local_note_1.abyte = -5;  local_note_1.bbyte = -5; 
  generic_array_insert_element(local_arr_1,5,&local_note_1);
  print_local_arr();

  i=generic_array_get_size(local_arr_1);
  printf("\n\n\ninserting element (-10,-10) one past last position, at pos %d \n",i);
  local_note_1.abyte = -10;  local_note_1.bbyte = -10; 
  generic_array_insert_element(local_arr_1,i,&local_note_1);
  print_local_arr();

  printf("\n\n\nappending element (101,102) \n");
  local_note_1.abyte = 101;  local_note_1.bbyte = 102; 
  generic_array_append_element(local_arr_1,&local_note_1);
  print_local_arr();

  printf("\n\n\nremoving element at position 0 \n");
  generic_array_remove_element(local_arr_1,0,0);
  print_local_arr();

  printf("\n\n\nremoving element at position 6 \n");
  generic_array_remove_element(local_arr_1,6,0);
  print_local_arr();

  i=generic_array_get_size(local_arr_1)-1;
  printf("\n\n\nremoving element at last position, pos %d \n",i);
  generic_array_remove_element(local_arr_1,i,&local_note_1);
  print_local_arr();
  printf("\n\nnote removed from element at position %d was ",i);
  print_my_midi_note(&local_note_1);
  printf("\n\n");




  printf("\n\ncleaning up\n\n");
  generic_array_destroy(local_arr_1);
  return 0;
}
