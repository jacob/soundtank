/*
 * soundtank internal commands code: change directory
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


void soundtank_command_cd(int argc, char** argv){
  rtobject_t* to_obj;

  if (argc < 2){
    curr_path = master_path;
    return;
  }

  if (!(to_obj = get_rtobject_from_path(argv[1]))){
    printf("cd error: could not find rtobject %s\n",argv[1]);
    return ;
  }

  if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(to_obj)){
    printf("cd error: target is not a signal path\n");
    return;
  }

  curr_path = (signal_path_t*)to_obj->imp_struct;
}


void soundtank_command_pwd(int argc, char** argv){
  rtobject_t* local_rtobj;
  char *ret;

  /*first find curr path's rtobject*/
  local_rtobj = get_rtobject_from_address(signal_path_get_owner_rtobject_address(curr_path));

  /*don't print master path's name, just a slash will do*/
  if (local_rtobj == master_path_rtobject){

    printf("/\n");
    return;

  }
  
  ret = rtobject_get_absolute_pathname(local_rtobj);

  printf("%s/\n", ret);
  
  free(ret);

}
