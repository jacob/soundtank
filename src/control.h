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

#ifndef SOUNDTANK_CONTROL_INCLUDE
#define SOUNDTANK_CONTROL_INCLUDE



#define RANGE_MIN_EXISTS 0x1
#define RANGE_DEFAULT_EXISTS 0x2
#define RANGE_MAX_EXISTS 0x4
#define RANGE_MIN_EXCLUSIVE 0x8 
#define RANGE_MAX_EXCLUSIVE 0x10
#define RANGE_INTEGER 0x20

typedef struct range range_t;
struct range{
  float min, def, max;
  int flags; /*range_flags:lowerbound exists,default exists,upperbound
               exists,lowerbound exclusive,upperbound
	       exclusive,integer values only*/
};



typedef struct control control_t;
struct control{
  
  /*Soundtank local stuff*/
  char* desc_string;
  int desc_code; /*Soundtank local code*/
  range_t linear_range;

  /*LADSPA stuff*/
  unsigned belongs_to_ladspa : 1; 
  unsigned long ladspa_port;
  int ladspa_hints; /*we keep them around for possible debugging*/
  unsigned input : 1; /*output controls are legal in LADSPA*/
};

typedef float control_instance_t;


control_t* control_alloc();
void control_dealloc(control_t* control);


const char* control_get_desc_string(const control_t* control);
int control_set_desc_string(control_t* control, const char *desc_string);

int control_get_desc_code(const control_t* control);
void control_set_desc_code(control_t* control, int desc_code);

float control_get_range_min(const control_t* control);
void control_set_range_min(control_t* control, float min);

float control_get_range_def(const control_t* control);
void control_set_range_def(control_t* control, float def);

float control_get_range_max(const control_t* control);
void control_set_range_max(control_t* control, float max);

int control_get_range_flags(const control_t* control);
void control_set_range_flags(control_t* control, int flags);

int control_belongs_to_ladspa(const control_t* control);
void control_set_belongs_to_ladspa(control_t* control, int belongs);

unsigned long control_get_ladspa_port(const control_t* control);
void control_set_ladspa_port(control_t* control, unsigned long ladspa_port);

int control_get_ladspa_hints(const control_t* control);
void control_set_ladspa_hints(control_t* control, int ladspa_hints);

int control_get_input(const control_t* control);
void control_set_input(control_t* control, int input);


control_t* control_create_from_desc_code(int desc_code);
control_t* control_create_from_ladspa_port(const LADSPA_PortDescriptor * port,\
					   const char * const port_name,\
					   const LADSPA_PortRangeHint * port_range_hint,\
					   unsigned long port_index);



#endif
