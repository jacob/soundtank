/*
 * rtobject control code
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

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <ladspa.h>

#include "include.h"
#include "soundtank_structs.h"

control_t* control_alloc(){
  control_t* new_control;

  if (!(new_control = (control_t*)malloc(sizeof(control_t)))){
    return 0;
  }

  new_control->desc_string = 0;
  new_control->desc_code = -1;
  new_control->linear_range.min = -1;
  new_control->linear_range.def = -1;
  new_control->linear_range.max = -1;
  new_control->linear_range.flags = -1;

  new_control->belongs_to_ladspa = 0;
  new_control->ladspa_port = 0;
  new_control->ladspa_hints = -1;
  new_control->input = 1;

  return new_control;
}


void control_dealloc(control_t* control){

  if (control->desc_string)
    free(control->desc_string);

  free(control);

}

const char* control_get_desc_string(const control_t* control){
  return control->desc_string;
}

int control_set_desc_string(control_t* control, const char *desc_string){

  if (control->desc_string)
    free(control->desc_string);

  if (!(control->desc_string = strdup(desc_string)))
    return -1;

  return 0;
}

int control_get_desc_code(const control_t* control){
  return control->desc_code;
}

void control_set_desc_code(control_t* control, int desc_code){
  control->desc_code = desc_code;
}

float control_get_range_min(const control_t* control){
  return control->linear_range.min;
}

void control_set_range_min(control_t* control, float min){
  control->linear_range.min = min;
}

float control_get_range_def(const control_t* control){
  return control->linear_range.def;
}

void control_set_range_def(control_t* control, float def){
  control->linear_range.def = def;
}

float control_get_range_max(const control_t* control){
  return control->linear_range.max;
}

void control_set_range_max(control_t* control, float max){
  control->linear_range.max = max;
}

int control_get_range_flags(const control_t* control){
  return control->linear_range.flags;
}

void control_set_range_flags(control_t* control, int flags){
  control->linear_range.flags = flags;
}

int control_belongs_to_ladspa(const control_t* control){
  if (control->belongs_to_ladspa)
    return 1;
  return 0;
}

void control_set_belongs_to_ladspa(control_t* control, int belongs){
  if (belongs)
    control->belongs_to_ladspa = 1;
  else
    control->belongs_to_ladspa = 0;
}

unsigned long control_get_ladspa_port(const control_t* control){
  return control->ladspa_port;
}

void control_set_ladspa_port(control_t* control, unsigned long ladspa_port){
  control->ladspa_port = ladspa_port;
}

int control_get_ladspa_hints(const control_t* control){
  return control->ladspa_hints;
}

void control_set_ladspa_hints(control_t* control, int ladspa_hints){
  control->ladspa_hints = ladspa_hints;
}
 
int control_get_input(const control_t* control){
  return control->input;
}

void control_set_input(control_t* control, int input){
  if (input)
    control->input = 0x1;
  else
    control->input = 0x0;
}


control_t* control_create_from_desc_code(int desc_code){
  control_t* new_control;

  new_control = 0;

  switch (desc_code){

  case CONTROL_DESC_ACTIVE:
    if (!(new_control = control_alloc()))
      return 0;
    if (control_set_desc_string(new_control, CONTROL_DESC_STRING_ACTIVE) < 0){
      control_dealloc(new_control);
      return 0;
    }
    control_set_desc_code(new_control, desc_code);
    control_set_input(new_control, 1);
    control_set_range_min(new_control, 0);
    control_set_range_def(new_control, 1);
    control_set_range_max(new_control, 1);
    control_set_range_flags(new_control, RANGE_MIN_EXISTS | \
			    RANGE_DEFAULT_EXISTS | RANGE_MAX_EXISTS |\
			    RANGE_INTEGER);
    break;
  case CONTROL_DESC_MUTE:
    if (!(new_control = control_alloc()))
      return 0;
    if (control_set_desc_string(new_control, CONTROL_DESC_STRING_MUTE) < 0){
      control_dealloc(new_control);
      return 0;
    }
    control_set_desc_code(new_control, desc_code);
    control_set_input(new_control, 1);
    control_set_range_min(new_control, 0);
    control_set_range_def(new_control, 0);
    control_set_range_max(new_control, 1);
    control_set_range_flags(new_control, RANGE_MIN_EXISTS | \
			    RANGE_DEFAULT_EXISTS | RANGE_MAX_EXISTS |\
			    RANGE_INTEGER);
    break;
  case CONTROL_DESC_VOLUME:
    if (!(new_control = control_alloc()))
      return 0;
    if (control_set_desc_string(new_control, CONTROL_DESC_STRING_VOLUME) < 0){
      control_dealloc(new_control);
      return 0;
    }
    control_set_desc_code(new_control, desc_code);
    control_set_input(new_control, 1);
    control_set_range_min(new_control, 0);
    control_set_range_def(new_control, 1);
    control_set_range_flags(new_control, RANGE_MIN_EXISTS | \
			    RANGE_DEFAULT_EXISTS);
    break;
  default:
    return 0;
  }

  return new_control;
}

control_t* control_create_from_ladspa_port(const LADSPA_PortDescriptor * port,
					   const char * const port_name,
					   const LADSPA_PortRangeHint * port_range_hint,
					   unsigned long port_index){
  control_t* new_control;
  int range_flags;
  float val;

  if (!(new_control = control_alloc()))
    return 0;

  if (control_set_desc_string(new_control, port_name) < 0){
    control_dealloc(new_control);
    return 0;
  }

  control_set_belongs_to_ladspa(new_control, 1);
  control_set_ladspa_port(new_control, port_index);
  control_set_ladspa_hints(new_control, port_range_hint->HintDescriptor);
  control_set_input(new_control, LADSPA_IS_PORT_INPUT(*port));

  /*we have to build up range flags one property at a time*/
  range_flags = 0;

  /*check for lower bound*/
  if (LADSPA_IS_HINT_BOUNDED_BELOW(port_range_hint->HintDescriptor)){
    range_flags |= RANGE_MIN_EXISTS;
    val = port_range_hint->LowerBound;
    if (LADSPA_IS_HINT_SAMPLE_RATE(port_range_hint->HintDescriptor))
      val *= soundtank_engine->sample_rate;
    control_set_range_min(new_control, val);
  }

  /*check for default*/
  if (LADSPA_IS_HINT_HAS_DEFAULT(port_range_hint->HintDescriptor)){
    range_flags |= RANGE_DEFAULT_EXISTS;
    if ((getLADSPADefault(port_range_hint,
			  soundtank_engine->sample_rate,
			  &val)) < 0){
      printf("LADSPA error: failed attempt to figure out default value\n");
    }else{
      control_set_range_def(new_control, val);
    }
  }

  /*check for upper bound*/
  if (LADSPA_IS_HINT_BOUNDED_ABOVE(port_range_hint->HintDescriptor)){
    range_flags |= RANGE_MAX_EXISTS;
    val = port_range_hint->UpperBound;
    if (LADSPA_IS_HINT_SAMPLE_RATE(port_range_hint->HintDescriptor))
      val *= soundtank_engine->sample_rate;
    control_set_range_max(new_control, val);
  }

  /*locally, we consider toggle to be an integer case (TODO: probably
    need to change)*/
  if (LADSPA_IS_HINT_TOGGLED(port_range_hint->HintDescriptor)){
    range_flags |= RANGE_INTEGER;
    control_set_range_min(new_control, 0);
    control_set_range_max(new_control, 1);
  }
  
  if (LADSPA_IS_HINT_INTEGER(port_range_hint->HintDescriptor))
    range_flags |= RANGE_INTEGER; 

  /*set range flags*/
  control_set_range_flags(new_control, range_flags);

  /*try to determine if this is a pitch control*/
  if ((LADSPA_IS_HINT_DEFAULT_440(port_range_hint->HintDescriptor))||\
      ((strstr(port_name,"Frequency")))||\
      ((strstr(port_name,"frequency")))||\
      ((strstr(port_name,"Pitch")))||\
      ((strstr(port_name,"pitch")))){

    control_set_desc_code(new_control, CONTROL_DESC_PITCH);

  }


  return new_control;
}


