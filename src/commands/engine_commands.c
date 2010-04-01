/*
 * soundtank internal commands code: engine control
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



void soundtank_command_start(int argc, char** argv){

  if (engine_get_state(soundtank_engine) == ENGINE_STATE_ACTIVE){

    printf("Engine already running\n");

  }else{

    if (engine_start(soundtank_engine) < 0 ){
      printf("Start engine error, attempt to start engine failed!\n");
    }else{
      printf("Engine Started\n");
    }

  }

}

void soundtank_command_stop(int argc, char** argv){

  if (engine_get_state(soundtank_engine) == ENGINE_STATE_ACTIVE){

    if (engine_stop(soundtank_engine) < 0){
      printf("Stop engine error, failed to stop engine!\n");
    }else{
      printf("Engine Stopped\n");
    }
	
  }else{

    printf("Engine is not running\n");

  }

}
