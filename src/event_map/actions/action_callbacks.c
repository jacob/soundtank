/*
 * event map action callbacks coordination code
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

void map_action_print_available_actions(){
  int i;

  for (i = 0; soundtank_action_callbacks[i].name; i++){
    
    printf("%s: %s [%s]\n\n",
           soundtank_action_callbacks[i].name,
           soundtank_action_callbacks[i].desc,
           soundtank_action_callbacks[i].args_desc);

  }

}




int map_action_init(ev_route_frame_t* frame, int argc, char **argv){
  int i;
  action_callback_desc *action_desc;

  /*need at least an action name*/
  if (argc < 1){
    printf("map action init error: not enough args\n");
    return -1;
  }

  /*allocate new action struct*/
  if (!(frame->action = map_action_alloca())){
    printf("map action init error: memory error\n");
    return -1;
  }

  /*find action type via callback fxn name*/
  action_desc = 0;
  for (i = 0; soundtank_action_callbacks[i].name; i++){

    if (strcmp(argv[0], soundtank_action_callbacks[i].name) == 0){

      action_desc = &soundtank_action_callbacks[i];

      break;
    }

  }

  if (!action_desc){
    printf("map action init error: couldn't find action %s\n", argv[0]);
    return -1;
  }

  /*init fxn*/
  frame->action->fxn = action_desc->func;

  /*call initialization fxn, if there is one*/
  if (action_desc->init){
    
    if (action_desc->init(frame, argc, argv) < 0){
      printf("map action init error: couldn't initialize action args\n");
      return -1;
    }
    
  }

  return 0;
}

char* map_action_get_func_name(map_action_t *action){
  int i;

  for (i = 0; soundtank_action_callbacks[i].name; i++){

    if (action->fxn == soundtank_action_callbacks[i].func)
      return strdup(soundtank_action_callbacks[i].name);

  }

  return 0;
}

char* map_action_get_argv(map_action_t *action, int arg_index, rtobject_t *rtobj){
  int i;
  char* ret;
  
  for (i = 0; soundtank_action_callbacks[i].name; i++){

    if (action->fxn == soundtank_action_callbacks[i].func){
      
      if (soundtank_action_callbacks[i].get_argv){
	ret = soundtank_action_callbacks[i].get_argv(action, arg_index, rtobj);
	if (ret) return ret;
	return 0;
      }else
	return 0;

    }
  }

  return 0;
}
