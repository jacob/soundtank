/*
 * soundtank internal commands code: move/remove
 *
 * Copyright 2003-2004 Jacob Robbins
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
#include <popt.h>

#include "../include.h"
#include "../soundtank_structs.h"



void soundtank_command_move_rtobject(int argc, char** argv){
  rtobject_t* obj;
  node_t* temp_node;
  signal_path_t* from_path;
  rtobject_t* to_path_rtobj;
  signal_path_t* to_path;

  from_path = 0;
  to_path = 0;

  if ((argc > 1)&&(!strcmp(argv[1],"--help"))){
    printf("useage: mv from-object [+|-|target-path] [before|after target-object]\n");
    printf("explanation: move an rtobject to a different signal path or position in the process order\n");
    printf("example: mv obj1 path1\n");
    printf("example: mv obj1 +++\n");
    printf("example: mv obj1 after obj2\n");
    return;
  }


  /*need at least an object and a destination*/
  if (argc < 3){
    printf("move error: not enough arguments\n");
    return;
  }
  
  /*find the object*/
  if (!(obj = get_rtobject_from_path(argv[1]))){
    printf("move error: could not find rtobject %s\n",argv[1]);
    return;
  }

  /*find current sigpath*/
  if (!(from_path = rtobject_get_parent_path(obj))){
    printf("move error: rtobject has no parent path\n");
    return;
  }

  /*find the destination*/
  if ((!strcmp(argv[2],"after"))||(!strcmp(argv[2],"before"))){
    /*move proximate to another object*/
    rtobject_t* target_obj;
    int to_pos;

    if (argc < 4){
      printf("move error: move %s what?\n",argv[2]);
      return;
    }

    /*find relative target*/
    if (!(target_obj = get_rtobject_from_path(argv[3]))){
      printf("move error: could not find destination object %s\n",argv[3]);
      return;
    }
    to_path = rtobject_get_parent_path(target_obj);

    /*calculate destination index*/
    to_pos = signal_path_get_position_from_node(to_path,signal_path_get_node(to_path,target_obj));
    if (!strcmp(argv[2],"before")) --to_pos;
    if (to_pos < 0) to_pos = 0;
	
    /*move, either local or inter-path*/
    if (to_path == from_path){

      if ((signal_path_move(from_path,signal_path_get_node(from_path,obj),1,to_pos)) < 0){
	printf("move error: move attempt failed\n");
	return;
      }

    }else{

      if ((signal_path_inter_path_move(from_path,signal_path_get_node(from_path,obj),1,to_path,to_pos)) < 0){
	printf("move error: move attempt failed\n");
	return;
      }

    }    


  }else if ((argv[2][0] == '+')||(argv[2][0] == '-')){
    /*relative moves*/
    int curr_pos, to_pos, i;

    if (!(temp_node = signal_path_get_node(from_path, obj))){
      printf("move error: can not find object to move in signal path it says it's in\n");
      return;
    }

    if ((curr_pos = signal_path_get_position_from_node(from_path, temp_node)) < 0){
      printf("move error: failed attempt to lookup current position\n");
      return;
    }

    to_pos = curr_pos;

    /*parse +'s and -'s in command line*/
    for (i=0;i<strlen(argv[2]);++i){
      if (argv[2][i] == '+') ++to_pos;
      if (argv[2][i] == '-') --to_pos;
    }

    /*check for short circuit*/
    if (to_pos == curr_pos) return;
    /*being not zero-based is a special consideration here*/
    if (to_pos < curr_pos) --to_pos;
    if (to_pos < 0) to_pos = 0; /*NOT ZERO BASED!*/

    /*move object*/
    if ((signal_path_move(from_path,temp_node,1,to_pos)) < 0){
      printf("move failed\n");
    }
    

  }else if ((to_path_rtobj = get_rtobject_from_path(argv[2]))){
    /*move to a signal path*/

    /*sanity check: verify target is a signal path*/
    if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(to_path_rtobj)){
      printf("move error: target is not a signal path\n");
      return;
    }

    to_path = (signal_path_t*)to_path_rtobj->imp_struct;

    /*check that target is not inside signal path to be moved*/
    if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(obj)){
      if (signal_path_get_node((signal_path_t*)obj->imp_struct, to_path_rtobj)){
	printf("move error: can not move signal path into one of its children\n");
	return;
      }
    }

    if (from_path == to_path) return; /*TODO fix this to append to path*/
    
    if (!(temp_node = signal_path_get_node(from_path,obj))){
      printf("move error: could not find object node in signal path\n");
      return;
    }
    
    if ((signal_path_inter_path_move(from_path, temp_node, 1, to_path, -1)) < 0){
      printf("move error: failed attempt to commit move\n");
      return;
    }

  }else{
    /*change name of rtobject*/
    int i,diff_path;

    /*see if new name is in a different signal path*/
    diff_path = 0;
    for (i=0;i<strlen(argv[2]);++i)
      if (argv[2][i] == '/') diff_path = 1;

    if (!diff_path){
      /*easy case: same path, different name*/
      if ((rtobject_set_name(obj,argv[2])) < 0){
	printf("mv error: memory error occured while trying to change name\n");
	return;
      }
      if (rtobject_update_alsa_seq_port_name(obj) < 0){
	printf("mv error: couldn't update ALSA seq port name\n");
      }
    }else{
      /*more work case: different path and different name*/
      char* to_path_name;
      char* new_name;

      to_path_name = pathname_get_path(argv[2]);
      new_name = pathname_get_name(argv[2]);
      
      /*find new path rtobject*/
      if (!(to_path_rtobj = get_rtobject_from_path(to_path_name))){
	printf("mover error: could not find destination path %s\n",to_path_name);
	free(new_name);
	free(to_path_name);
	return;
      }

      /*verify new path is a path*/
      if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(to_path_rtobj)){
	printf("move error: destination path %s does not exist\n",to_path_name);
	free(new_name);
	free(to_path_name);
	return;
      }
      
      /*get new path*/
      to_path = (signal_path_t*)to_path_rtobj->imp_struct;

      /*move to new path*/
      if ((signal_path_inter_path_move(from_path, signal_path_get_node(from_path,obj), \
				       1, to_path, -1)) < 0){
	printf("move error: failed attempt to commit move\n");
	free(new_name);
	free(to_path_name);
	return;
      }

      /*change name to new name*/
      if ((rtobject_set_name(obj,new_name)) < 0){
	printf("move error: failed attempt to change name but did move to new path\n");
	free(new_name);
	free(to_path_name);
	return;
      }
      if (rtobject_update_alsa_seq_port_name(obj) < 0){
	printf("mv error: couldn't update ALSA seq port name\n");
      }

      free(new_name);
      free(to_path_name);

    }

  }

  /*remake the process list to enact changes*/
  if ((remake_process_list()) < 0){
    printf("move error: failed while remaking process list, DATA CORRUPTION\n");
  }

}

