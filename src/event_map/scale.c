/*
 * musical scale object code
 *
 * Copyright 2004 Jacob Robbins
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <math.h>

#include "../include.h"



float rt_note_to_pitch(const scale_t* scale, unsigned char note){
  int octave;
  int note_index;
  float note_multiplier;

  /*find octave & note_index*/
  if (note >= scale->base_note){

    octave = (note - scale->base_note) / generic_array_get_size(scale->note_list);

  }else{

    octave = (note - scale->base_note - generic_array_get_size(scale->note_list) + 1)\
      / generic_array_get_size(scale->note_list);

  }

  /*find note position in scale*/
  note_index = note - scale->base_note - (octave * generic_array_get_size(scale->note_list));

  /*get scale's note multiplier corresponding to note position*/
  note_multiplier = *((float*)generic_array_get_element_pointer(scale->note_list, note_index));

  /*return base_pitch * note * (2 ^ octave) */
  return scale->base_pitch * note_multiplier * pow(2, octave);
}

void scale_print(const scale_t* scale){
  int i;

  printf("%s ", scale_get_name(scale));
  printf("%d notes ( %d : %f ) { ", \
	 scale_get_note_list_size(scale),\
	 scale_get_base_note(scale),\
	 scale_get_base_pitch(scale));

  for (i=0;i<scale_get_note_list_size(scale);++i){
    if (i>0) printf(", ");
    printf("%f", scale_get_note(scale, i));
  }

  printf(" }");

}

scale_t* create_scale(const int argc, const char **argv){
  int i;
  scale_t* new_scale;

  /*need at least 4 args: name, base note, base pitch, and one or more
    scale notes*/
  if (argc < 4){
    printf("create scale error: not enough args\n");
    return 0;
  }

  /*make the scale*/
  if (!(new_scale = scale_alloc())){
    printf("create scale error: memory error\n");
    return 0;
  }

  /*set the name*/
  if (scale_set_name(new_scale, argv[0]) < 0){
    printf("create scale error: couldn't set name to %s\n", argv[0]);
    scale_dealloc(new_scale);
    return 0;
  }

  /*set base note*/
  if (!(string_is_number(argv[1]))){
    printf("create scale error: base note (2nd arg) must be a number, ");
    printf("you entered %s\n", argv[1]);
    scale_dealloc(new_scale);
    return 0;
  }
  scale_set_base_note(new_scale, atoi(argv[1]));

  /*set base pitch*/
  if (!(string_is_number(argv[2]))){
    printf("create scale error: base pitch (3rd arg) must be a number, ");
    printf("you entered %s\n", argv[2]);
    scale_dealloc(new_scale);
    return 0;
  }
  scale_set_base_pitch(new_scale, atof(argv[2]));
  
  /*add the scale notes*/
  for (i=3;i<argc;++i){

    if (!(string_is_number(argv[i]))){
      printf("create scale error: note %d must be a number, ", i-2);
      printf("you entered %s\n", argv[i]);
      scale_dealloc(new_scale);
      return 0;
    }
  
    if (scale_append_note(new_scale, atof(argv[i])) < 0){
      printf("create scale error: failed to add note %d\n", i-2);
      scale_dealloc(new_scale);
      return 0;
    }
      
  }

  return new_scale;
}

scale_t* scale_alloc(){
  scale_t* new_scale;
  
  if (!(new_scale = (scale_t*)malloc(sizeof(scale_t)))){
    return 0;
  }

  new_scale->name = 0;
  new_scale->base_pitch = 0;
  new_scale->base_note = 0;
  
  if (!(new_scale->note_list = generic_array_create(sizeof(float)))){
    free(new_scale);
    return 0;
  }

  return new_scale;
}

void scale_dealloc(scale_t* scale){
  if (scale->name) free(scale->name);
  generic_array_destroy(scale->note_list);
  free(scale);
}

const char* scale_get_name(const scale_t* scale){
  return scale->name;
}

int scale_set_name(scale_t* scale, const char *name){
 
  if (scale->name) free(scale->name);

  if (!(scale->name = strdup(name))){
    return -1;
  }

  return 0;
}

unsigned char scale_get_base_note(const scale_t* scale){
  return scale->base_note;
}

void scale_set_base_note(scale_t* scale, unsigned char base_note){
  scale->base_note = base_note;
}

float scale_get_base_pitch(const scale_t* scale){
  return scale->base_pitch;
}

void scale_set_base_pitch(scale_t* scale, float base_pitch){
  if (base_pitch > 0)
    scale->base_pitch = base_pitch;
  else
    scale->base_pitch = base_pitch * -1;
}

int scale_get_note_list_size(const scale_t* scale){
  return generic_array_get_size(scale->note_list);
}

float scale_get_note(const scale_t* scale, int pos){
  float *ret;
  ret = (float*)generic_array_get_element_pointer(scale->note_list, pos);
  if (ret)
    return *ret;
  return -1;
}

int scale_insert_note(scale_t* scale, float new_note, int pos){
  /*NOT ZERO BASED!! <0->append, ==0->prepend, ==1->after 1st element...*/

  if (pos < 0){
    /*append*/
    
    if (generic_array_append_element(scale->note_list, &new_note) < 0)
      return -1;

  }else{

    if (generic_array_insert_element(scale->note_list, pos, &new_note) < 0)
      return -1;

  }

  return 0;
}


int scale_append_note(scale_t* scale, float new_note){
  if (generic_array_append_element(scale->note_list, &new_note) < 0)
    return -1;
  return 0;
}

int scale_save_to_xml(const scale_t* scale, xmlNodePtr* xml_node){
  
  return 0;
}

scale_t* scale_load_from_xml(xmlNodePtr* xml_node){

  return 0;
}
