/*
 * event map test callbacks coordination code
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

#include "../../include.h"

void map_test_print_available_tests(){
  int i;

  for (i = 0; soundtank_test_callbacks[i].name; i++){
    
    printf("%s: %s [%s]\n\n",
	   soundtank_test_callbacks[i].name,
	   soundtank_test_callbacks[i].desc,
	   soundtank_test_callbacks[i].args_desc);

  }

}


int map_test_init(ev_route_frame_t* frame, int argc, char **argv){
  int i;
  test_callback_desc *test_desc;

  /*need at least a test name*/
  if (argc < 1){
    printf("map test init error: not enough args\n");
    return -1;
  }

  /*find test type via callback fxn name*/
  test_desc = 0;
  for (i = 0; soundtank_test_callbacks[i].name; i++){

    if (strcmp(argv[0], soundtank_test_callbacks[i].name) == 0){

      test_desc = &soundtank_test_callbacks[i];

      break;
    }

  }

  if (!test_desc){
    printf("map test init error: couldn't find test %s\n", argv[0]);
    return -1;
  }

  /*init fxn*/
  frame->test->fxn = test_desc->func;

  /*init args*/
  if (argc > 1){

    /*make sure there is an initialization fxn*/
    if (test_desc->init){

      if (test_desc->init(frame, argc, argv) < 0){
	printf("map test init error: couldn't initialize test args\n");
	return -1;
      }

    }

  }

  return 0;
}

char* map_test_get_func_name(const map_test_t *test){
  int i;

  for (i = 0; soundtank_test_callbacks[i].name; i++){

    if (test->fxn == soundtank_test_callbacks[i].func)
      return strdup(soundtank_test_callbacks[i].name);

  }

  return 0;
}

char* map_test_get_argv(const map_test_t *test, int arg_index, rtobject_t *rtobj){
  int i;

  for (i = 0; soundtank_test_callbacks[i].name; i++){

    if (test->fxn == soundtank_test_callbacks[i].func){
      
      if (soundtank_test_callbacks[i].get_argv)
	return soundtank_test_callbacks[i].get_argv(test, arg_index, rtobj);
      else
	return 0;

    }
  }

  return 0;
}

