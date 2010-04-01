/*
 * automated event map creation code
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

#include "../include.h"



int rtobject_create_auto_map(rtobject_t* rtobj){
  int pitch_control, i;
  ev_route_frame_t frame;
  map_test_t test_struct;
  char *pitch_control_string;

  /*initialize variables including pesky test structure*/
  frame.rtobj = rtobj;
  memset((void*)&test_struct, 0, sizeof(map_test_t));
  frame.test = &test_struct;

  /*see if there is a pitch control*/
  pitch_control = -1;
  for (i=0;i<rtobject_get_control_list_size(rtobj);++i){
    const control_t* curr_control = rtobject_get_control(rtobj, i);

    /*sanity check*/
    if (!curr_control)
      return -1;

    /*this is set when the control is created in control.c*/
    if (control_get_desc_code(curr_control) == CONTROL_DESC_PITCH){
      pitch_control = i;
      break;
    }

  }

  /*make an instrument-type setup*/
  if (pitch_control >= 0){

    /*put the index of the pitch control into a string*/
    if (!(pitch_control_string = (char*)malloc(16 * sizeof(char)))){
      printf("auto map error: memory error/n");
      return -1;
    }
    sprintf(pitch_control_string, "%d", pitch_control);

    /*create the map*/
    if (!(frame.map = event_map_alloca())){
      printf("auto map error: couldn't create new map/n");
      return -1;
    }

    /*note-on test*/
    map_test_clear(&test_struct);
    {
      char * pass[] = { "type", "noteon" };
      
      /*initialize test (tests aren't allocated)*/
      if (map_test_init(&frame, 2, pass) < 0){
	printf("auto map error: couldn't create noteon test\n");
	event_map_dealloca(frame.map);
	return -1;
      }
   
      /*note-on test, action 0 : set note flag*/
      {
	char * action_pass[] = { "flag", "note", "on" };

	if (map_action_init(&frame, 3, action_pass) < 0){
	  printf("auto map error: couldn't create flag action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      
	if (map_test_insert_action(frame.test, frame.action, -1) < 0){
	  printf("auto map error: couldn't insert action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      }
      
      /*note-on test, action 1 : set pitch using default scale*/
      {
	char * action_pass[] = { "pitch", pitch_control_string,\
				 "def", "match" };

	if (map_action_init(&frame, 4, action_pass) < 0){
	  printf("auto map error: couldn't create flag action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      
	if (map_test_insert_action(frame.test, frame.action, -1) < 0){
	  printf("auto map error: couldn't insert action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      }

      /*note-on test, action 2 : activate instance*/
      {
	char * action_pass[] = { "set", "0",\
				 "const", "1", "match" };

	if (map_action_init(&frame, 5, action_pass) < 0){
	  printf("auto map error: couldn't create flag action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      
	if (map_test_insert_action(frame.test, frame.action, -1) < 0){
	  printf("auto map error: couldn't insert action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      }

      /*put test into map*/
      if (event_map_insert_test(frame.map, frame.test, -1) < 0){
	printf("auto map error: couldn't insert noteon test\n");
	event_map_dealloca(frame.map);
	return -1;
      }
    }

    /*create the note-off test*/
    memset((void*)&test_struct, 0, sizeof(map_test_t));
    {
      char * pass[] = { "type", "noteoff" };
      
      if (map_test_init(&frame, 2, pass) < 0){
	printf("auto map error: couldn't create noteoff test\n");
	event_map_dealloca(frame.map);
	return -1;
      }

      /*note-off test, action 0 : deactivate instance*/
      {
	char * action_pass[] = { "set", "0",\
				 "const", "0", "match" };

	if (map_action_init(&frame, 5, action_pass) < 0){
	  printf("auto map error: couldn't create flag action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      
	if (map_test_insert_action(frame.test, frame.action, -1) < 0){
	  printf("auto map error: couldn't insert action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      }
      
      /*note-off test, action 1 : unset the note flag*/
      {
	char * action_pass[] = { "flag", "note", "off" };

	if (map_action_init(&frame, 3, action_pass) < 0){
	  printf("auto map error: couldn't create flag action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      
	if (map_test_insert_action(frame.test, frame.action, -1) < 0){
	  printf("auto map error: couldn't insert action\n");
	  event_map_dealloca(frame.map);
	  return -1;
	}
      }
      
      /*put test into map*/
      if (event_map_insert_test(frame.map, frame.test, -1) < 0){
	printf("auto map error: couldn't insert noteoff test\n");
	event_map_dealloca(frame.map);
	return -1;
      }
    }

    /*stick map in rtobject*/
    if (rtobject_insert_map(rtobj, frame.map, 0) < 0){
      printf("auto map error: couldn't insert new map into rtobject\n");
      event_map_dealloca(frame.map);
      return -1;
    }

  }

  return 0;
}

