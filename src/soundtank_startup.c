/*
 * Soundtank startup & shutdown code
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <popt.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/mman.h>

#include "include.h"
#include "soundtank_structs.h"


/*local var*/
extern char dyn_command[128]; /*current command-line*/
char dyn_command_1[128]; /*previous command*/
char dyn_command_2[128]; /* ... */
char dyn_command_3[128];
char command_line_prefix[128]; /*thing that goes at beginning of each line*/

int init_msg_shm(){

  /*NOTE: 585 is always used as the id of the soundtank inter-process
    message queue. If another application also uses id 585, serious
    problems will result*/

  if ((msgid = msgget(585,IPC_CREAT|0666))<0) return -1;

  /*NOTE: the following avoids the possibility of another application
    interfering... but runs out of msg queue id's when soundtank is
    used too many times:*/

  /*
  if ((msgid = msgget(IPC_PRIVATE,0666))<0)
    return -1;
  */

  return 0;
}

void free_msg_shm(){
  msgctl(msgid, IPC_RMID, 0);
}
  
void soundtank_cleanup(){

  if (soundtank_engine->state == ENGINE_STATE_ACTIVE) 
    engine_stop(soundtank_engine);

  /*save command history to file*/
  soundtank_save_command_history(50);

  free_msg_shm();

  if (alsa_seq_client_handle){
    
    if ((snd_seq_close(alsa_seq_client_handle)) < 0)
      printf("WARNING: failed attempt to close ALSA sequencer client\n");

  }else{

    printf("WARNING: no ALSA seq client handle at shutdown\n");

  }

}

int soundtank_init(int argc, const char** argv){
  int i, err, env_ok;

  /********/
  /*Options That Mean Don't Actually Run Soundtank*/
  /********/
  for (i = 0; i < argc; ++i){
    if ((!strcmp(argv[i],"-v"))||(!strcmp(argv[i],"--version"))){
      printf("Soundtank Version %s\n", SOUNDTANK_APP_VERSION);
      exit(0);
    }
  }

  /********/
  /*System & Libraries Initialization*/
  /********/

  /*initialize internal command line completion*/
  rl_attempted_completion_function = soundtank_completion;

  /*initialize gnu history library*/
  using_history();
  soundtank_load_command_history();

  /*set command line prefix internal variable*/
  strcpy(command_line_prefix,"sndtnk");

  /*set LADSPA_PATH environment variable to default if it is not set already*/
  if ((setenv("LADSPA_PATH","/home/<user>/.ladspa:/usr/local/lib/ladspa:/usr/lib/ladspa", 0)) < 0){
    printf("\nWarning: not enough memory to set LADSPA PATH environment variable\n");
  }

  /*init msg shm, used for IPC communication with rt thread*/
  if ((init_msg_shm())<0) {
    printf("\nError could not initialize internal msg shm\n"); 
    return -1;
  }

  /*try to lock all application memory into RAM*/
  if (mlockall(MCL_CURRENT | MCL_FUTURE) < 0){
    printf("\nCouldn't lock Soundtank into RAM, you're probably not running as root ");
    printf("and Soundtank doesn't support libcap yet.\n\n");
  }

  /*create an ALSA sequencer client*/
  if ((snd_seq_open(&alsa_seq_client_handle, "default",\
		    SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK)) < 0){
    printf("ERROR: could not create an ALSA sequencer client\n");
    printf("no other way to control objects so aborting\n");
    return -1;
  }

  snd_seq_set_client_name(alsa_seq_client_handle, "Soundtank");

 

  /********/
  /* Soundtank Application Core Structs Initialization*/
  /********/

  /*init process list*/
  process_list = 0;
  process_list_size = 0;
  swap_process_list = 0;
  swap_process_list_size = 0;

  /*init addressing list*/
  rtobject_address_list_size = 0;
  rtobject_address_list = 0;
  swap_rtobject_address_list_size = 0;
  swap_rtobject_address_list = 0;

  /*init buffer list*/
  buffer_list = 0;

  /*init scale list*/
  scale_list = 0;

  /*set rt-error readout mode to default*/
  rt_error_readout = RT_ERROR_READOUT_LOUD;

  /*zero out process debug var*/
  debug_process_function = 0;

  /*allocate engine*/
  if (!(soundtank_engine = engine_alloca())){
    printf("\ncould not allocate memory for engine\n");
    return -1;
  }

  /*initialize engine*/
  if ((err = engine_initialize(soundtank_engine, argc, argv)) != 0){
    printf("error initializing engine: (%d)\n",err);
    return -1;
  }



  /********/
  /* Soundtank Environment Initialization*/
  /* Stage 1: Mandatory Elements*/
  /********/

  /*create null channels for unconnected ports*/
  {
    buffer_t* null_read_buffer;
    buffer_t* null_write_buffer;
      
    /*null read channel is always empty, unconnected input ports are
      attached to this*/
    if (!(null_read_buffer = get_free_buffer())){
      printf("\n could not allocate buffer for disconnected input ports\n");
      return -1;
    }
    if ((buffer_add_reference(null_read_buffer, -333, BUFF_SHARE)) < 0){
      printf("\n could not lock access to buffer for disconnected input ports\n");
      return -1;
    }
    if (!(null_read = channel_alloca(CHANNEL_SCOPE_GLOBAL))){
      printf("\ncould not allocate channel for disconnected input ports\n");
      return -1;
    }
    if ((channel_set_buffer(null_read, null_read_buffer, -1)) < 0){
      printf("\ncould not initialize channel for disconnected input ports\n");
      return -1;
    }
    
    /*null write channel is filled with junk that's never read,
      unconnected output ports are attached to this*/
    if (!(null_write_buffer = get_free_buffer())){
      printf("\n could not allocate buffer for disconnected output ports\n");
      return -1;
    }
    if ((buffer_add_reference(null_write_buffer,-666,BUFF_SHARE)) < 0){
      printf("\n could not lock access to buffer for disconnected output ports\n");
      return -1;
    }
    if (!(null_write = channel_alloca(CHANNEL_SCOPE_GLOBAL))){
      printf("\ncould not allocate channel for disconnected output ports\n");
      return -1;
    }
    if ((channel_set_buffer(null_write, null_write_buffer, -1)) < 0){
      printf("\ncould not initialize channel for disconnected output ports\n");
      return -1;
    }
    
  }

  /*create master path*/
  {
    if ((err = create_rtobject(RTOBJECT_MAJOR_TYPE_SIGNAL_PATH, RTOBJECT_IMP_TYPE_SIGNAL_PATH,\
			     0, 0, "main", "_the_ master path", 0)) < 0){
      printf("fatal initialization error: could not create rtobject for master path\n");
      return -1;
    }
    
    /*store location of master path rtobject*/
    if (!(master_path_rtobject = get_rtobject_from_address(err))){
      printf("fatal initialization error: could not locate master path rtobject\n");
      return -1;
    }
  
    /*store location of master signal path imp object*/
    master_path = (signal_path_t*)master_path_rtobject->imp_struct;
    
    /*set current path to master path*/
    curr_path = master_path;
    
  }

  /*create top level signal paths: in, src, efx, out*/
  if ((err = create_rtobject(RTOBJECT_MAJOR_TYPE_SIGNAL_PATH, RTOBJECT_IMP_TYPE_SIGNAL_PATH, \
			       0, 0, "in", "", curr_path))<0){
    printf("fatal initialization error: could not create path 'in'\n");
    return -1;
  }

  if ((err = create_rtobject(RTOBJECT_MAJOR_TYPE_SIGNAL_PATH, RTOBJECT_IMP_TYPE_SIGNAL_PATH, \
			       0, 0, "src", "", curr_path))<0){
    printf("fatal initialization error: could not create path 'src'\n");
    return -1;
  }

  if ((err = create_rtobject(RTOBJECT_MAJOR_TYPE_SIGNAL_PATH, RTOBJECT_IMP_TYPE_SIGNAL_PATH, \
			       0, 0, "efx", "", curr_path))<0){
    printf("fatal initialization error: could not create path 'efx'\n");
    return -1;
  }

  if ((err = create_rtobject(RTOBJECT_MAJOR_TYPE_SIGNAL_PATH, RTOBJECT_IMP_TYPE_SIGNAL_PATH, \
			       0, 0, "out", "", curr_path))<0){
    printf("fatal initialization error: could not create path 'out'\n");
    return -1;
  }

  /********/
  /* Soundtank Environment Initialization*/
  /* Stage 2: Non-Mandatory Elements*/
  /********/

  env_ok = 1;

  /*add default scale, "def"*/
  /*TODO: put this in environment init file, ~/.soundtank_env*/
  {
    scale_t *def_scale;
    const char *def_scale_argv[] = { "def", "69", "440", "1", "1.0546875",\
				     "1.125", "1.2", "1.25", "1.3333333",\
				     "1.40625", "1.5", "1.6", "1.6875",\
				     "1.8", "1.875" };
    
    if (!(def_scale = create_scale(15, def_scale_argv))){
      printf("\nerror couldn't create default scale\n");
      env_ok = 0;
    }

    if (add_scale(def_scale) < 0){
      printf("\n error couldn't add default scale to Soundtank's scale list\n");
      env_ok = 0;
    }
    
  }


  /*load contents of environment init file, ~/.soundtank_env, 
    make a default one if none exists*/
  {
    int env_pathname_len;
    char *ret, *env_pathname;
    const char* env_filename = ".soundtank_env";

    /*get user's home directory from environment*/
    if (!(ret = getenv("HOME"))){
      printf("fatal initialization error: couldn't get home directory from shell environment variables, you must set the the HOME variable\n");
      return -1;
    }

    /*find how long absolute pathname of Soundtank environment file must be*/
    env_pathname_len = strlen(ret) + strlen(env_filename) + 2; /*+1 terminating null, +1 '/'*/

    /*allocate and fill environment file pathname*/
    if (!(env_pathname = (char*)malloc(env_pathname_len * sizeof(char)))){
      printf("fatal initialization error: memory error\n");
      return -1;
    }
    strcpy(env_pathname, ret);
    env_pathname[strlen(ret)] = '/';
    strcpy(&env_pathname[strlen(ret) + 1], env_filename);

    /*see if Soundtank environment file exists yet, if not make a default one*/
    {
      xmlDocPtr doc;
      
      printf("checking for Soundtank environment file, ~/.soundtank_env\n");
      if (!(doc = xmlParseFile(env_pathname))){

	if (errno == ENOENT){
	  /*environment file doesn't exist so make one*/
	  
	  /*make default external outs*/
	  printf("making a default set of external outputs\n");
	  if (soundtank_make_default_extern_outs() < 0){
	    printf("initialization error: failed to make default extern outs\n");
	  }

	  /*save env file*/
	  printf("saving current state into environment file, ~/.soundtank_env\n");
	  if (rtobject_save_to_file(master_path_rtobject, env_pathname) < 0){
	    printf("fatal initialization error: unable to write default environment file\n");
	    return -1;
	  }
	  
	}else{
	  printf("initialization error: environment file corrupt ~/.soundtank_env\n");
	  return -1;
	}
	
      }else{

	/*load Soundtank environment file (~/.soundtank_env)*/
	printf("loading environment file, ~/.soundtank_env\n");
	if (load_soundtank_xml_file(env_pathname) < 0){
	  printf("fatal initialization error: unable to load Soundtank environment file, ~/.soundtank_env\n");
	  return -1;
	}

	/*if no extern outs have been created, make some default ones*/
	/*TODO: stick them in the environment file*/
	if (!(get_rtobject_from_path("/out/out0"))){
	
	  printf("\nNOTE: your environment file (~/.soundtank_env) contains no extern outs\n");
	  printf("for the selected engine method, ");
	  if (engine_get_method(soundtank_engine) == ENGINE_METHOD_ALSA)
	    printf("ALSA, ");
	  else if (engine_get_method(soundtank_engine) == ENGINE_METHOD_JACK)
	    printf("JACK, ");
	  printf("so a default set of extern outs will be created\n");
	  
	  if (soundtank_make_default_extern_outs() < 0){
	    printf("initialization error: failed to make default extern outs\n");
	  }

	}
	
      }
    
    }

    
    free(env_pathname);
    
  }

  if (!env_ok){
    printf("WARNING: an error occured while initializing environment\n");
    printf("Soundtank may be missing major functionality, aborting\n");
    printf("review startup output for error details\n");
    return -1;
  }



  /********/
  /* Soundtank Post-Environment Initialization*/
  /********/

  /*set current path to /src path*/
  {
    rtobject_t* src_path_rtobject;

    if ((src_path_rtobject = signal_path_get_rtobject_from_name(master_path,"src"))){
      curr_path = (signal_path_t*)(src_path_rtobject)->imp_struct;
    }else{
      printf("no /src path was found\n");
    }

  }

  /*turn engine on*/
  printf("starting engine...   ");
  if (engine_start(soundtank_engine) < 0){
    printf("\nerror starting engine\n");
    return -1;
  }
   
  /*tell user if the engine is live*/
  if (engine_get_state(soundtank_engine) == ENGINE_STATE_ACTIVE){
    printf("audio engine is live!\n");
  }else{
    printf("audio engine is not running\n");
  }

  /*welcome & Copyright notice*/
  printf("\n\n  Welcome to Soundtank, released under the GNU Public License\n");
  printf("  Copyright 2003-2004 Jacob Robbins\n\n\n");

  return 0;
}

int soundtank_make_default_extern_outs(){
  int i, num_outs, err;
  rtobject_t* out_rtobj;
  signal_path_t* out_path;
  
  /*find the /out signal path to put the extern outs in*/
  out_rtobj = get_rtobject_from_path("/out");
  out_path = (signal_path_t*)out_rtobj->imp_struct;
  
  /*determine how many outs to make*/
  if (engine_get_method(soundtank_engine) == ENGINE_METHOD_ALSA)
    num_outs = soundtank_engine->num_channels;
  else if (engine_get_method(soundtank_engine) == ENGINE_METHOD_JACK)
    num_outs = 2;
  else{
    printf("make default extern outs error: invalid engine type\n");
    return -1;
  }
  
  /*make extern outs*/
  for (i = 0; i < num_outs; ++i){
    char* argv[1];
    char channel_name[32];
    char channel_number[32];
    
    sprintf(channel_number, "%d", i);
    sprintf(channel_name, "out%d", i);
    argv[0] = channel_number;
    
    if (debug_readout)
      printf("\nmaking extern out object named %s\n", channel_name);
    
    if (engine_get_method(soundtank_engine) == ENGINE_METHOD_ALSA){
      if ((err = create_rtobject(RTOBJECT_MAJOR_TYPE_EXTERN_OUT,\
				 RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT, \
				 1, (const char **)argv, channel_name,\
				 "", out_path)) < 0 ){
	printf("\n error creating extern out rtobject #%d\n",i);
	break;
      }
    }else if (engine_get_method(soundtank_engine) == ENGINE_METHOD_JACK){
      if ((err = create_rtobject(RTOBJECT_MAJOR_TYPE_EXTERN_OUT,\
				 RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT, \
				 0, 0, channel_name,\
				 "", out_path)) < 0 ){
	printf("\n error creating extern out rtobject #%d\n",i);
	break;
      }
    }
  }
  
  return 0;
}

int soundtank_get_command(){
  char *line, *prefix;
  const char *curr_path_name;
  int prefix_len;
  
  /*make copy of complete prefix string*/
  curr_path_name = rtobject_get_name(get_rtobject_from_address(signal_path_get_owner_rtobject_address(curr_path)));

  prefix_len = 4 + strlen(command_line_prefix) + strlen(curr_path_name);

  if (!(prefix = malloc(prefix_len * sizeof(char)))){
    printf("get command error: memory error\n");
    return 0;
  }
  
  snprintf(prefix, prefix_len, "%s %s> ", command_line_prefix, curr_path_name);

  /*get next line via gnu readline*/
  line = readline(prefix);

  free(prefix);

  if ((line)&&(strcmp(line,""))){

    /*don't put quit command into the history file*/
    if ((strcmp(line,"q"))&&(strcmp(line,"quit")))
      add_history(line);
    strncpy(dyn_command,line,128);
    free(line);

  }else{

    dyn_command[0]='\0';
    return 0;
  }
  
  return 0;
} 

int soundtank_load_command_history(){
  int hist_pathname_len;
  char *ret, *hist_pathname;
  const char* hist_filename = ".soundtank_history";
  
  /*get user's home directory from environment*/
  if (!(ret = getenv("HOME"))){
    printf("save history error: couldn't get home directory from shell environment variables, you must set the the HOME variable\n");
    return -1;
  }
  
  /*find how long absolute pathname of Soundtank environment file must be*/
  hist_pathname_len = strlen(ret) + strlen(hist_filename) + 2; /*+1 terminating null, +1 '/'*/
  
  /*allocate and fill environment file pathname*/
  if (!(hist_pathname = (char*)malloc(hist_pathname_len * sizeof(char)))){
    printf("save history error: memory error\n");
    return -1;
  }
  strcpy(hist_pathname, ret);
  hist_pathname[strlen(ret)] = '/';
  strcpy(&hist_pathname[strlen(ret) + 1], hist_filename);

  if (read_history(hist_pathname) != 0){
    /*no history file, start one*/
    add_history("");
    write_history(hist_pathname);
  }

  return 0;
}

int soundtank_save_command_history(int numlines){
  int hist_pathname_len;
  char *ret, *hist_pathname;
  const char* hist_filename = ".soundtank_history";
  
  /*get user's home directory from environment*/
  if (!(ret = getenv("HOME"))){
    printf("save history error: couldn't get home directory from shell environment variables, you must set the the HOME variable\n");
    return -1;
  }
  
  /*find how long absolute pathname of Soundtank environment file must be*/
  hist_pathname_len = strlen(ret) + strlen(hist_filename) + 2; /*+1 terminating null, +1 '/'*/
  
  /*allocate and fill environment file pathname*/
  if (!(hist_pathname = (char*)malloc(hist_pathname_len * sizeof(char)))){
    printf("save history error: memory error\n");
    return -1;
  }
  strcpy(hist_pathname, ret);
  hist_pathname[strlen(ret)] = '/';
  strcpy(&hist_pathname[strlen(ret) + 1], hist_filename);

  /*call GNU history library to actually save to file*/
  if (append_history(numlines, hist_pathname) != 0){
    printf("save history error: couldn't append to history file\n");
    return -1;
  }

  /*truncate file to numlines*/
  if (history_truncate_file(hist_pathname, numlines) != 0){
    printf("save history error: couldn't truncate history file\n");
    return -1;
  }

  return 0;
}

char** soundtank_completion (const char* text, int start, int end)
{
  /*code adapted from example in gnu readline docs*/
  char **matches;

  matches = (char **)NULL;

  /*complete w/ rtobject names in current path*/
  matches = rl_completion_matches(text, rtobject_name_generator);

  return (matches);
}

char* rtobject_name_generator(const char* text, int state)
{
  /*code adapted from example in gnu readline docs*/
  static int len;
  static int offset;
  static node_t* curr_node;
  rtobject_t* curr_obj;

  /* If this is a new word to complete, initialize now.  This
     includes saving the length of TEXT for efficiency, and
     initializing the index ptr to the start of the specified path's 
     object list. */
  if (!state){
    int i;
    char *path_name;
    rtobject_t* path_rtobj;
    signal_path_t* path;

    len = strlen (text);

    /*error check for empty case*/
    if (!len) return 0;
    
    /*check for prefix path in text being completed*/
    for (i = len - 1; i >= 0; --i)
      if (text[i] == '/') break;

    offset = i;

    if (offset >= 0){
      
      /*case where a prefix path must be resolved*/

      /*dup the text being completed and cut off everything after the prefix path*/
      if (!(path_name = strdup(text))){
	printf("internal memory error\n");
	return  0;
      }
      path_name[offset + 1] = '\0';

      /*look up prefix path rtobject*/
      path_rtobj = get_rtobject_from_path(path_name);
      
      /*done with duplicate*/
      free(path_name);

      /*verify rtobject from prefix path is signal path and get the sigpath imp object*/
      if ((!path_rtobj)||\
	  (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(path_rtobj)))
	return 0; /*invalid path in text to be completed*/
      path = (signal_path_t*)path_rtobj->imp_struct;


    }else{

      /*case where current path is specified implicitly*/
      path = curr_path;

    }

    /*initialize the index pointer to the start of the specified path*/
    curr_node = path->object_list;
  }

  /* Return the next rtobject name which partially matches from the contents
     of the specified path */
  while (curr_node){
    curr_obj = (rtobject_t*)curr_node->data;
      
    /*increment index*/
    curr_node=curr_node->next;

    /*check current object's name against text after prefix*/
    if (strncmp(rtobject_get_name(curr_obj), &text[offset + 1], len - offset - 1) == 0){

      if (offset >= 0){

	/*make a new string to hold prefix & completed name*/
	char *ret = (char*)malloc((3 + offset + strlen(rtobject_get_name(curr_obj))) * sizeof(char));
	if (!ret) return 0;

	/*copy prefix*/
	strncpy(ret, text, offset + 1);

	/*copy completed name*/
	strncpy(&ret[offset + 1],rtobject_get_name(curr_obj),strlen(rtobject_get_name(curr_obj)));

	/*add terminating null char and fwd slash if signal path*/
	if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(curr_obj)){
	  ret[offset + strlen(rtobject_get_name(curr_obj)) + 1] = '/';
	  ret[offset + strlen(rtobject_get_name(curr_obj)) + 2] = '\0';
	}else
	  ret[offset + strlen(rtobject_get_name(curr_obj)) + 1] = '\0';

	return ret;

      }else{

	return strdup(rtobject_get_name(curr_obj));

      }
      
    }

  }

  /* If no rtobject names matched, then return NULL. */
  return 0;
}












/* TODO remove everything below this line */

void reprint_command_line(){
  printf("%s %s> %s",command_line_prefix,rtobject_get_name(get_rtobject_from_address(signal_path_get_owner_rtobject_address(curr_path))),dyn_command);
}  

void print_err_msg(int err_code, char* msg){
  if (err_code == SOUNDTANK_RT_ERROR){
    printf("\n a realtime error has occured\n");
  }else{
    printf("a playback error has occured\n");
  }

}
