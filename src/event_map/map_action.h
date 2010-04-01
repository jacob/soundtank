/*
 * event map action element code
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


#ifndef EVENT_MAP_ACTION_INCLUDE
#define EVENT_MAP_ACTION_INCLUDE


/*from event_map.h*/
typedef struct ev_route_frame ev_route_frame_t;

typedef struct map_action map_action_t;

typedef int (*map_action_callback_t)(snd_seq_event_t *ev, ev_route_frame_t *frame);

typedef int (*map_action_init_t)(ev_route_frame_t* frame, int argc, char **argv);

typedef char* (*map_action_get_argv_t)(map_action_t* action, int arg_index, rtobject_t *rtobj);

typedef enum map_action_args_type map_action_args_type_t;
enum map_action_args_type{
  
  int_args,
  float_args,
  set_args,
  pitch_args

};

struct map_action{

  union{

    struct{

      int arg1,arg2,arg3;

    }int_args;

    struct{
      
      float arg1,arg2,arg3;

    }float_args;

    struct{

      int control_index;
      int ev_param;
      float fixed_value;
      unsigned match : 1;
      unsigned shift : 1;
      float shift_min, shift_max;

    }set_args;

    struct{

      int control_index;
      scale_t* scale;
      unsigned match : 1;

    }pitch_args;

  }args;

  map_action_callback_t fxn;

};

void map_action_print(map_action_t* action, rtobject_t *rtobj);

map_action_t* map_action_alloca();
void map_action_dealloca(map_action_t* action);
map_action_t* map_action_copy(map_action_t* src); /*allocates memory*/


int map_action_save_to_xml(map_action_t* action, xmlNodePtr* xml_node, rtobject_t *rtobj);
map_action_t* map_action_load_from_xml(xmlNodePtr* xml_node, rtobject_t* rtobj);


#endif
