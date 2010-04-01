/*
 * soundtank internal commands code: make & modify event maps
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


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <popt.h>

#include "../include.h"
#include "../soundtank_structs.h"



void soundtank_command_map(int argc, char** argv){
  rtobject_t* rtobj;
  int i;

  /*handle help message*/
  for (i=0;i<argc;++i){
    if ((!strcmp(argv[i],"-h"))||(!(strcmp(argv[i],"--help")))){
      printf("\n");
      printf("explanation: map creates & modifies an rtobject's event maps\n");
      printf("useage: map rtobject [count|list|insert|delete|append|flush] args\n");
      printf("count: no args, shows how many event maps rtobject has\n");
      printf("list: no args, show all event maps in rtobject\n");
      printf("[insert|delete] position: create or remove event map at position\n");
      printf("append: no args, create a map at the end of rtobject's event map list\n");
      printf("flush: no args, delete all of rtobject's event maps\n");
      printf("auto: no args, try to automagically create and fill event map\n");
      printf("\n");
      return;
    }
  }

  if (argc < 3){
    printf("map error: not enough arguments\n");
    return;
  }

  if (!(rtobj = get_rtobject_from_path(argv[1]))){
    printf("map error: could not find target object, %s\n",argv[1]);
    return;
  }
 
  if ((!strcmp(argv[2],"count"))||(!strcmp(argv[2],"-c"))){
    printf("%d\n", rtobject_get_map_list_size(rtobj));
  }else

  if ((!strcmp(argv[2],"list"))||(!strcmp(argv[2],"-l"))){
    event_map_t* curr_map;

    for (i=0;i<rtobject_get_map_list_size(rtobj);++i){

      if (rtobject_get_map(rtobj, &curr_map, i) < 0){
	printf("map list error: couldn't get map %d\n", i);
	return;
      }
      
      printf("%s map #%d:\n",rtobject_get_name(rtobj),i);
      event_map_print(curr_map, rtobj);

    }

  }else

  if ((!strcmp(argv[2],"insert"))||(!strcmp(argv[2],"-i"))){
    event_map_t* new_map;
    int pos;

    if (argc < 4){
      printf("map error: not enough arguments (insert at what position?)\n");
      return;
    }

    pos = atoi(argv[3]);

    if (!(new_map = event_map_alloca())){
      printf("map error: failed attempt to create new map\n");
      return;
    }

    if ((rtobject_insert_map(rtobj, new_map, pos)) < 0){
      printf("map error: failed attempt to insert map\n");
      event_map_dealloca(new_map);
      return;
    }

  }else

  if ((!strcmp(argv[2],"delete"))||(!strcmp(argv[2],"-d"))){
    int pos;

    if (argc < 4){
      printf("map error: not enough arguments (delete map at what position?)\n");
      return;
    }

    pos = atoi(argv[3]);

    if ((rtobject_delete_map(rtobj, pos)) < 0){
      printf("map error: failed attempt to delete map at position %d\n", pos);
      return;
    }

  }else

  if ((!strcmp(argv[2],"append"))||(!strcmp(argv[2],"-a"))){
    event_map_t* new_map;

    if (!(new_map = event_map_alloca())){
      printf("map error: failed attempt to create new map\n");
      return;
    }

    if ((rtobject_append_map(rtobj, new_map)) < 0){
      printf("map error: failed attempt to append map\n");
      event_map_dealloca(new_map);
      return;
    }

  }else

  if ((!strcmp(argv[2],"flush"))||(!strcmp(argv[2],"-f"))){

    rtobject_flush_map_list(rtobj);

  }else

  if ((!strcmp(argv[2],"auto"))||(!strcmp(argv[2],"-m"))){

    if (rtobject_create_auto_map(rtobj) < 0){
      printf("map error: failed attempt to create auto map\n");
    }

  }else

  {    
    printf("map error: unknown command: %s\n",argv[2]);
    return;
  }

}

void soundtank_command_test(int argc, char** argv){
  ev_route_frame_t frame;
  rtobject_t* rtobj;
  int i, map_index;
  event_map_t *map;


  /*handle help message*/
  for (i=0;i<argc;++i){
    if ((!strcmp(argv[i],"-h"))||(!(strcmp(argv[i],"--help")))){
      printf("explanation: test creates & modifies an rtobject's event map's tests\n");
      printf("useage: test rtobject map-index [count|list|insert|delete|append|flush] [args]\n");
      printf("map-index: a number giving the position of map in the rtobject's map list\n");
      printf("count: no args, shows how many tests the event map has\n");
      printf("list: no args, show all tests in event map\n");
      printf("insert position test-type args : create a test-type test with args at position\n");
      printf("delete position: no args, delete test at position\n");
      printf("append test-type args: create test-type test with args & append to map\n");
      printf("flush: no args, delete all of event map's tests\n");
      printf("(note: to list all test types, do 'test show-tests')\n");
      return;
    }
  }

  if ((argc > 1)&&(!(strcmp("show-tests",argv[1])))){
    map_test_print_available_tests();
    return;
  }

  if (argc < 4){
    printf("test error: not enough arguments\n");
    return;
  }

  if (!(rtobj = get_rtobject_from_path(argv[1]))){
    printf("test error: could not find target object: %s\n",argv[1]);
    return;
  }
 
  /*make sure the map index is a number*/
  for (i=0;i<strlen(argv[2]);++i){
    if (!(isdigit(argv[2][i]))){
      printf("test error: map index must be a number, you entered %s\n",argv[2]);
      return;
    }
  }

  map_index = atoi(argv[2]);

  /*the only reason we do this is to validate that the map exists (get rid of it?)*/
  if ((rtobject_get_map(rtobj,&map,map_index)) < 0){
    printf("test error: could not find map #%d\n", map_index);
    return;
  }

  if ((!strcmp(argv[3],"count"))||(!strcmp(argv[3],"-c"))){
    printf("%d\n",rtobject_get_test_list_size(rtobj,map_index));
  }else

  if ((!strcmp(argv[3],"list"))||(!strcmp(argv[3],"-l"))){
    event_map_print(map, rtobj);
  }else

  if ((!strcmp(argv[3],"insert"))||(!strcmp(argv[3],"-i"))){
    map_test_t new_test;

    /*this is a hassle, the hassle of dealing with structures directly
      in arrays as opposed to using arrays of pointers to
      structures. the motivation is that it is faster to have all test
      structures in the same array as they are not big, but maybe this
      should be dumped in favor of using a uniform implementation
      style for map lists, maps, tests & actions*/
    memset((void*)&new_test, 0, sizeof(map_test_t));

    if (argc < 6){
      printf("insert test error: not enough args: need test position and test type\n");
      return;
    }

    frame.rtobj = rtobj;
    frame.map = map;
    frame.test = &new_test;
       

    if (map_test_init(&frame, argc - 5, &argv[5] ) < 0){
      printf("failed attempt to initialize test\n");
      return;
    }

    /*put it in the map*/
     if ((rtobject_insert_test(rtobj, &new_test, map_index, \
			       atoi(argv[4]))) < 0){
      printf("insert test error: failed attempt to insert new test\n");
      return;
    }

  }else

  if ((!strcmp(argv[3],"delete"))||(!strcmp(argv[3],"-d"))){

    if (argc < 5){
      printf("test delete error: not enough args, delete test at what position?\n");
      return;
    }

    if ((rtobject_delete_test(rtobj, map_index, atoi(argv[4]))) < 0){
      printf("test delete error: failed attempt to delete test %s\n", argv[4]);
      return;
    }

  }else

  if ((!strcmp(argv[3],"append"))||(!strcmp(argv[3],"-a"))){
    map_test_t new_test;

    /*this is a hassle, the hassle of dealing with structures directly
      in arrays as opposed to using arrays of pointers to
      structures. the motivation is that it is faster to have all test
      structures in the same array as they are not big, but maybe this
      should be dumped in favor of using a uniform implementation
      style for map lists, maps, tests & actions*/
    memset((void*)&new_test, 0, sizeof(map_test_t));

    if (argc < 5){
      printf("append test error: not enough args, need test type\n");
      return;
    }

    frame.rtobj = rtobj;
    frame.map = map;
    frame.test = &new_test;

    if (map_test_init(&frame, argc - 4, &argv[4] ) < 0){
      printf("failed attempt to initialize test\n");
      return;
    }

    /*put it in the map*/
    if ((rtobject_append_test(rtobj, &new_test, map_index)) < 0){
      printf("append test error: failed attempt to append new test\n");
      return;
    }


  }else

  if ((!strcmp(argv[3],"flush"))||(!strcmp(argv[3],"-f"))){

    rtobject_flush_test_list(rtobj, map_index);

  }else

  {
    printf("test error: unknown command: %s\n", argv[3]);
    return;
  }

}


void soundtank_command_action(int argc, char** argv){
  ev_route_frame_t frame;
  rtobject_t* rtobj;
  int i, map_index, test_index;
  event_map_t *map;
  map_test_t* test;

  /*handle help message*/
  for (i=0;i<argc;++i){
    if ((!strcmp(argv[i],"-h"))||(!(strcmp(argv[i],"--help")))){
      printf("explanation: action creates & modifies actions\n");
      printf("(event maps are lists of tests, each test has a list of actions)\n");
      printf("useage: action rtobject map-index test-index [count|list|insert|delete|append|flush] [args]\n");
      printf("map-index: a number giving the position of map in the rtobject's map list\n");
      printf("test-index: a number giving the position of the test in the map\n");
      printf("count: no args, show how many actions the test has\n");
      printf("list: no args, show all actions the test has\n");
      printf("insert position action-type args : create a action-type action with args at position\n");
      printf("delete position: no args, delete action at position\n");
      printf("append action-type args: create action-type action with args & append to test's action list\n");
      printf("flush: no args, delete all of test's actions\n");
      printf("(note: to list all action types, do 'action show-actions')\n");
      return;
    }
  }

  if ((argc > 1)&&(!(strcmp("show-actions",argv[1])))){
    map_action_print_available_actions();
    return;
  }

  if (argc < 5){
    printf("action error: not enough arguments\n");
    return;
  }

  if (!(rtobj = get_rtobject_from_path(argv[1]))){
    printf("action error: could not find target object: %s\n",argv[1]);
    return;
  }
 
  /*make sure the map index is a number*/
  for (i=0;i<strlen(argv[2]);++i){
    if (!(isdigit(argv[2][i]))){
      printf("action error: map index must be a number, you entered %s\n",argv[2]);
      return;
    }
  }

  map_index = atoi(argv[2]);

  /*the only reason we do this is to validate that the map exists (get rid of it?)*/
  if ((rtobject_get_map(rtobj, &map, map_index)) < 0){
    printf("action error: could not find map #%d\n", map_index);
    return;
  }

  /*make sure the test index is a number*/
  for (i=0; i<strlen(argv[3]); ++i){
    if (!(isdigit(argv[3][i]))){
      printf("action error: test index must be a number, you entered %s\n",argv[3]);
      return;
    }
  }

  test_index = atoi(argv[3]);

  /*the only reason we do this is to validate that the test exists (get rid of it?)*/
  if ((rtobject_get_test(rtobj, &test, map_index, test_index)) < 0){
    printf("action error: could not find test #%d\n", test_index);
    return;
  }


  if ((!strcmp(argv[4],"count"))||(!strcmp(argv[4],"-c"))){
    printf("%d\n",rtobject_get_action_list_size(rtobj, map_index, test_index));
  }else

  if ((!strcmp(argv[4],"list"))||(!strcmp(argv[4],"-l"))){
    printf("fxn not written yet\n");
  }else

  if ((!strcmp(argv[4],"insert"))||(!strcmp(argv[4],"-i"))){

    if (argc < 7){
      printf("insert action error: not enough args: need action position and action type\n");
      return;
    }

    frame.rtobj = rtobj;
    frame.map = map;
    frame.test = test;

    if (map_action_init(&frame, argc - 6, &argv[6] ) < 0){
      printf("failed attempt to initialize action\n");
      return;
    }

    /*put it in the map*/
     if ((rtobject_insert_action(frame.rtobj, frame.action, map_index, \
				 test_index, atoi(argv[5]))) < 0){
      printf("insert action error: failed attempt to insert new action\n");
      return;
    }

  }else

  if ((!strcmp(argv[4],"delete"))||(!strcmp(argv[4],"-d"))){

    if (argc < 6){
      printf("action delete error: not enough args, delete action at what position?\n");
      return;
    }

    if ((rtobject_delete_action(rtobj, map_index,\
				test_index, atoi(argv[5]))) < 0){
      printf("action delete error: couldn't delete action %s\n", argv[5]);
      return;
    }

  }else

  if ((!strcmp(argv[4],"append"))||(!strcmp(argv[4],"-a"))){

    if (argc < 6){
      printf("append action error: not enough args: need action type\n");
      return;
    }

    frame.rtobj = rtobj;
    frame.map = map;
    frame.test = test;

    if (map_action_init(&frame, argc - 5, &argv[5] ) < 0){
      printf("failed attempt to initialize action\n");
      return;
    }

    /*put it in the map*/
     if (rtobject_append_action(frame.rtobj, frame.action, map_index, \
				 test_index) < 0){
      printf("insert action error: failed attempt to insert new action\n");
      return;
    }

  }else

  if ((!strcmp(argv[4],"flush"))||(!strcmp(argv[4],"-f"))){

    rtobject_flush_action_list(rtobj, map_index, test_index);

  }else

  {
    printf("action error: unknown command: %s\n", argv[4]);
    return;
  }

}


void soundtank_command_scale(int argc, char** argv){
  int i;

  /*handle help message*/
  for (i=0;i<argc;++i){
    if ((!strcmp(argv[i],"-h"))||(!(strcmp(argv[i],"--help")))){
      printf("explanation: scale creates & modifies scales\n");
      printf("useage: scale [count|list|add|remove] [args]\n");
      printf("add: name base-note base-pitch notes\n");
      printf("remove: pos or name\n");
      return;
    }
  }

  if (argc < 2){
    printf("scale error: not enough args\n");
    return;
  }

  if ((!strcmp(argv[1],"count"))||(!strcmp(argv[1],"-c"))){
    printf("%d scales\n",get_scale_list_size());
  }else

  if ((!strcmp(argv[1],"list"))||(!strcmp(argv[1],"-l"))){
    int i;
    scale_t* curr_scale;

    for (i=0;i<get_scale_list_size();++i){
      curr_scale = get_scale(i);
      scale_print(curr_scale);
      printf("\n");
    }

  }else

  if ((!strcmp(argv[1],"add"))||(!strcmp(argv[1],"-a"))){
    scale_t* new_scale;

    if (argc < 3){
      printf("scale add error: not enough args\n");
      return;
    }

    if (!(new_scale = create_scale(argc-2, (const char**)&argv[2]))){
      printf("add scale error: couldn't create scale\n");
      return;
    }

    if (add_scale(new_scale) < 0){
      printf("add scale error: couldn't store new scale in list\n");
      scale_dealloc(new_scale);
      return;
    }

  }else

  if ((!strcmp(argv[1],"remove"))||(!strcmp(argv[1],"-r"))){

    if (argc < 3){
      printf("remove scale error: not enough args, which scale?\n");
      return;
    }

    if (string_is_number(argv[2])){
      
      if (remove_scale(atoi(argv[2])) < 0){
	printf("remove scale error: failed to remove scale\n");
	return;
      }

    }else{

      if (remove_scale_by_name(argv[2]) < 0){
	printf("remove scale error: failed to remove scale\n");
	return;
      }

    }

    }else
  
    {
    printf("scale error: unrecognized action %s\n",argv[1]);
    }

}
