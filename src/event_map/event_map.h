/*
 * event map code
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


#ifndef EVENT_MAP_MAIN_INCLUDE
#define EVENT_MAP_MAIN_INCLUDE


/*note: typedef is stashed in map_action.h*/
struct ev_route_frame{

  rtobject_t* rtobj;
  event_map_t* map;
  map_test_t* test;
  map_action_t* action;
  char flags;

};

struct event_map{

  /*note: test array is terminated by an empty entry*/
  generic_array_t* test_array;

};


event_map_t* event_map_alloca();
void event_map_dealloca(event_map_t* map);
event_map_t* event_map_copy(event_map_t* src); /*allocates memory*/

int event_map_get_test_list_size(const event_map_t* map);
map_test_t* event_map_get_test(const event_map_t* map, int pos);
int event_map_insert_test(event_map_t* map, map_test_t* test, int pos);
/*NOT ZERO BASED!! <0->append, ==0->prepend, ==1->after 1st element...*/
int event_map_delete_test(event_map_t* map, int pos);
void event_map_clear_test_list(event_map_t* map);

void event_map_print(event_map_t* map, rtobject_t* rtobj);

int event_map_save_to_xml(const event_map_t* map, xmlNodePtr* xml_node,\
			  rtobject_t *rtobj);
event_map_t* event_map_load_from_xml(xmlNodePtr* xml_node, rtobject_t* rtobj);

/*realtime function; must not allocate memory*/
void rt_event_map_route_event(snd_seq_event_t* ev);

/*rtobject functions*/
int rtobject_get_map_list_size(rtobject_t* rtobj);
int rtobject_get_map(rtobject_t* rtobj, event_map_t** ptr, int pos);
int rtobject_insert_map(rtobject_t* rtobj, event_map_t* map, int pos);
int rtobject_delete_map(rtobject_t* rtobj, int pos);
int rtobject_append_map(rtobject_t* rtobj, event_map_t* map);
void rtobject_flush_map_list(rtobject_t* rtobj);

int rtobject_get_test_list_size(rtobject_t* rtobj, int map_index);
int rtobject_get_test(rtobject_t* rtobj, map_test_t** ptr, int map_index, int pos);
int rtobject_insert_test(rtobject_t* rtobj, map_test_t* test, int map_index, int pos);
int rtobject_delete_test(rtobject_t* rtobj, int map_index, int pos);
int rtobject_append_test(rtobject_t* rtobj, map_test_t* test, int map_index);
void rtobject_flush_test_list(rtobject_t* rtobj, int map_index);


int rtobject_get_action_list_size(rtobject_t* rtobj, int map, int test_pos);
int rtobject_get_action(rtobject_t* rtobj, map_action_t** ptr, int map, int test_pos, int pos);
int rtobject_insert_action(rtobject_t* rtobj, map_action_t* action, int map, int test_pos, int pos);
int rtobject_delete_action(rtobject_t* rtobj, int map, int test_pos, int pos);
int rtobject_append_action(rtobject_t* rtobj, map_action_t* action, int map, int test_pos);
void rtobject_flush_action_list(rtobject_t* rtobj, int map, int test_pos);


/*in auto_map.c*/
int rtobject_create_auto_map(rtobject_t* rtobj);


#endif
