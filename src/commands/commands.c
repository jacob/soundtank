/*
 * soundtank internal commands code
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
#include <popt.h>

#include "../include.h"
#include "../soundtank_structs.h"


void soundtank_print_interactive_help_message();


int soundtank_execute_command(const char* command_line){
  int command_argc,i;
  char *command_argv[36];


  /*init command argv array (only used for command line completion)*/
  for (i=0;i<36;++i){
    if (!(command_argv[i] = (char*)malloc(128*sizeof(char)))){
      printf("\nError could not allocate memory for command-line\n");
      return -1;
    }
  }
  
  /*parse command into argv style array*/
  if ((command_argc = string_to_argv_noalloc(command_line, command_argv, 36 ))<0){
    printf("\nexecute command error: internal failure while parseing command\n");
  }

  /*ignore empty command*/
  if (!command_argc) return 0;

  /*determine which fxn to call from zeroeth argument*/

  if ((!strcmp(command_argv[0],"q"))||(!strcmp(command_argv[0],"quit"))||(!strcmp(command_argv[0],"exit"))){
    printf("\nSoundtank is exiting\n\n");
    return -1;
  }
  
  if ((!strcmp(command_argv[0],"h"))||(!strcmp(command_argv[0],"help"))||(!strcmp(command_argv[0],"H"))){
    soundtank_print_interactive_help_message();
  }else
    
  if ((!strcmp(command_argv[0],"s"))||(!strcmp(command_argv[0],"stat"))){
    soundtank_command_stat(command_argc,command_argv);
  }else
      
  if ((!strcmp(command_argv[0],"ls"))||(!strcmp(command_argv[0],"list"))){
    soundtank_command_ls(command_argc, command_argv);
  }else

  if ((!strcmp(command_argv[0],"lsd"))||(!strcmp(command_argv[0],"lsd"))){
    printf("yeah\n");
  }else

  if ((!strcmp(command_argv[0],"cd"))||(!strcmp(command_argv[0],"cd"))){
    soundtank_command_cd(command_argc, command_argv);
  }else

  if ((!strcmp(command_argv[0],"pwd"))||(!strcmp(command_argv[0],"pwd"))){
    soundtank_command_pwd(command_argc, command_argv);
  }else

  if ((!strcmp(command_argv[0],"halt"))||(!strcmp(command_argv[0],"stop"))){
    soundtank_command_stop(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"begin"))||(!strcmp(command_argv[0],"start"))){
    soundtank_command_start(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"debug"))||(!strcmp(command_argv[0],"debug"))){
    soundtank_command_debug(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"i"))||(!strcmp(command_argv[0],"info"))){
    engine_print_info(soundtank_engine,command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"create"))||(!strcmp(command_argv[0],"cr"))){
    soundtank_command_create_rtobject(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"copy"))||(!strcmp(command_argv[0],"cp"))){
    soundtank_command_copy_rtobject(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"rm"))||(!strcmp(command_argv[0],"remove"))){
    soundtank_command_free_rtobject(command_argc,command_argv);
  }else
	
  if ((!strcmp(command_argv[0],"move"))||(!strcmp(command_argv[0],"mv"))){
    soundtank_command_move_rtobject(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"attach"))||(!strcmp(command_argv[0],"at")||(!strcmp(command_argv[0],"att")))){
    soundtank_command_attach_rtobject(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"detach"))||(!strcmp(command_argv[0],"dt"))||(!strcmp(command_argv[0],"det"))){
    soundtank_command_detach_rtobject(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"set"))||(!strcmp(command_argv[0],"set"))){
    soundtank_command_set(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"activate"))||(!strcmp(command_argv[0],"act"))||(!strcmp(command_argv[0],"a"))){
    soundtank_command_activate(command_argc,command_argv);
  }else
	  
  if ((!strcmp(command_argv[0],"deactivate"))||(!strcmp(command_argv[0],"deact"))||(!strcmp(command_argv[0],"da"))){
    soundtank_command_activate(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"mute"))||(!strcmp(command_argv[0],"m"))){
    soundtank_command_mute(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"unmute"))||(!strcmp(command_argv[0],"um"))||(!strcmp(command_argv[0],"u"))){
    soundtank_command_mute(command_argc,command_argv);
  }else
	  
  if ((!strcmp(command_argv[0],"volume"))||(!strcmp(command_argv[0],"vol"))||(!strcmp(command_argv[0],"v"))){
    soundtank_command_volume(command_argc,command_argv);
  }else
     
  if ((!strcmp(command_argv[0],"save"))||(!strcmp(command_argv[0],"sv"))){
    soundtank_command_save_rtobject(command_argc,command_argv);
  }else
 
  if ((!strcmp(command_argv[0],"load"))||(!strcmp(command_argv[0],"ld"))){
    soundtank_command_load_rtobject(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"map"))||(!strcmp(command_argv[0],"maps"))){
    soundtank_command_map(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"test"))||(!strcmp(command_argv[0],"tst"))){
    soundtank_command_test(command_argc,command_argv);
  }else

  if ((!strcmp(command_argv[0],"action"))||(!strcmp(command_argv[0],"act"))){
    soundtank_command_action(command_argc,command_argv);
  }else
	
  if ((!strcmp(command_argv[0],"scale"))||(!strcmp(command_argv[0],"sca"))){
    soundtank_command_scale(command_argc,command_argv);
  }else
	
  {
    printf("unrecognized command: %s\n",command_argv[0]);
  }

  /*free command argv array*/
  for (i=0;i<36;++i) free(command_argv[i]);

  
  /*in quiet rt-error readout mode, show rt-errors after each command*/
  /*this is similar to bash readout of completed jobs*/

  /*we must keep track of the rt-error count in order to check*/
  {
    static long long last_xrun_count = 0;
    
    if (last_xrun_count < soundtank_engine->total_xruns){
      if (rt_error_readout == RT_ERROR_READOUT_QUIET)
	printf("%lld realtime errors have occured\n",\
	       soundtank_engine->total_xruns - last_xrun_count);
      last_xrun_count = soundtank_engine->total_xruns;
    }

  }

  return 0;
}


void soundtank_print_interactive_help_message(){

  printf("\nsoundtank interactive commandline help:\n\n");
  printf("'q'  or 'quit' or 'exit' : exit program\n");
  printf("'h'  or 'help' : display this message\n");
  printf("'s'  or 'stat' : display current state of realtime audio engine\n");
  printf("'ls' or 'list' : display objects currently loaded in realtime audio engine\n");
  printf("'ls --help' : display help on using ls function\n");
  printf("'halt' or 'stop' : halt the realtime audio engine\n");
  printf("'begin' or 'start' : activate the realtime audio engine\n");
  printf("'i' or 'info' : display engine setup info\n");
  printf("'cr [OPTIONS] major_type name [imp_type] [imp args...]' : create new rtobject\n");
  printf("'mv rtobject signal-path|new-name' : move rtobject to a signal path or new name\n");
  printf("'rm rtobject' : remove/delete rtobject from soundtank\n");
  printf("'attach (or att) rtobject [port] target [target-port]' : attach port to target\n");
  printf("'detach' (or dt) rtobject [port]' : detach port/object from any channels\n");
  printf("'set rtobject [instance-#] control-# value' : set a control of an rtobject to a value\n");
  printf("'activate|act|a' : activate rtobject's instances\n");
  printf("'deactivate|deact|da'  : deactivate rtobject's instances\n");
  printf("'mute|m rtobject' : mute an rtobject's outputs\n");
  printf("'unmute|um|u rtobject' : unmute an rtobject's outputs\n");
  printf("'volume|vol|v rtobject' : set volume of an rtobject's outputs\n");
  printf("'cd signal-path' : change current directory to a different signal path\n");
  printf("'pwd' : print working directory (ie which signal path you're in)\n");
  printf("'save|sv rtobject filename' : save rtobject as xml doc filename\n");
  printf("'load|ld filename' : load xml doc filename\n");
  printf("'map|test|action rtobject [args]' : fxns to make & modify event maps for MIDI control\n");
  printf("\n");
  printf("\n");

}






