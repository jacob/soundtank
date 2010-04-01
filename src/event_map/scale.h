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

#ifndef SOUNDTANK_SCALE_OBJECT_INCLUDE
#define SOUNDTANK_SCALE_OBJECT_INCLUDE

/*NOTE: typedef is in rtobject.h for now, MOVE IT*/

struct scale{
  char *name;
  unsigned char base_note;
  float base_pitch;
  generic_array_t* note_list;
};


float rt_note_to_pitch(const scale_t* scale, unsigned char note);

void scale_print(const scale_t* scale);

scale_t* create_scale(const int argc, const char **argv);

scale_t* scale_alloc();
void scale_dealloc(scale_t* scale);

const char* scale_get_name(const scale_t* scale);
int scale_set_name(scale_t* scale, const char *name);

unsigned char scale_get_base_note(const scale_t* scale);
void scale_set_base_note(scale_t* scale, unsigned char base_note);

float scale_get_base_pitch(const scale_t* scale);
void scale_set_base_pitch(scale_t* scale, float base_pitch);

int scale_get_note_list_size(const scale_t* scale);
float scale_get_note(const scale_t* scale, int pos);

int scale_insert_note(scale_t* scale, float new_note, int pos);
/*NOT ZERO BASED!! <0->append, ==0->prepend, ==1->after 1st element...*/
int scale_append_note(scale_t* scale, float new_note);

int scale_save_to_xml(const scale_t* scale, xmlNodePtr* xml_node);
scale_t* scale_load_from_xml(xmlNodePtr* xml_node);



#endif
