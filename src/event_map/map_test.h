/*
 * event map test element code
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


#ifndef EVENT_MAP_TEST_INCLUDE
#define EVENT_MAP_TEST_INCLUDE


typedef struct map_test map_test_t;

typedef int (*map_test_callback_t)(snd_seq_event_t* ev, ev_route_frame_t* frame);

typedef int (*map_test_init_t)(ev_route_frame_t* frame, int argc, char **argv);

typedef char* (*map_test_get_argv_t)(const map_test_t* test, int arg_index, rtobject_t *rtobj);


struct map_test{
  
  int arg1,arg2,arg3;

  map_test_callback_t fxn;

  ll_head action_list;

};


void map_test_clear(map_test_t* test);
void map_test_print(const map_test_t* test, rtobject_t *rtobj);
void map_test_copy(map_test_t* dest, const map_test_t* src); 
/*does not allocate dest but does allocate copied actions*/

int map_test_get_action_list_size(const map_test_t* test);
map_action_t* map_test_get_action(const map_test_t* test, int pos);
int map_test_insert_action(map_test_t* test, map_action_t* action, int pos);
/*NOT ZERO BASED!! <0->append, ==0->prepend, ==1->after 1st element...*/
int map_test_delete_action(map_test_t* test, int pos);
void map_test_clear_action_list(map_test_t* test);


int map_test_save_to_xml(const map_test_t* test, xmlNodePtr* xml_node, rtobject_t *rtobj);
map_test_t* map_test_load_from_xml(xmlNodePtr* xml_node, rtobject_t* rtobj);


#endif
