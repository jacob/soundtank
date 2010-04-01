/*
 * soundtank internal commands code: operation statistics
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


void soundtank_command_stat(int argc,char** argv){
 
  printf("Engine is currently: ");
  engine_print_state(soundtank_engine);
  printf("\n");

  printf("Elapsed Frames: %lld\n",soundtank_engine->elapsed_frames);
  if (soundtank_engine->total_xruns){
    printf("Total xruns: %lld\n",soundtank_engine->total_xruns);
    printf("Last xrun Occured at Frame: %lld\n",soundtank_engine->last_xrun);
  }else{
    printf("No xruns have occured\n");
  }

}
