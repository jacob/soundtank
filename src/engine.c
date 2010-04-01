/*
 * Soundtank engine code
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
#include <stdio.h>
#include <popt.h>
 
#include "include.h"
#include "soundtank_structs.h"



engine_t* engine_alloca(){
  engine_t* new_struct;

  if (!(new_struct = (engine_t*)malloc(sizeof(engine_t)))) return 0;
  new_struct->state = ENGINE_STATE_NOT_READY;
  new_struct->method = ENGINE_METHOD_NONE;
  new_struct->device_name=0;
  new_struct->sample_size=0;
  new_struct->buffer_size=0;
  new_struct->sample_rate=0;
  new_struct->num_channels=0;
  new_struct->interleaved=0;
  new_struct->num_periods=0;
  new_struct->period_size=0;
  new_struct->avail_min=0;
  new_struct->elapsed_frames=0;
  new_struct->last_xrun=0;
  new_struct->total_xruns=0;

  if (!(new_struct->write_buffer_array = generic_array_create(sizeof(pair_i_v_t)))) {
    fprintf (stderr, "cannot create hardware buffer array\n");
    free(new_struct);
    return 0;
  }
  new_struct->buffer=0;
  return new_struct;
}

void engine_dealloca(engine_t* eng){
  if (eng->device_name) free(eng->device_name);
  if (eng->write_buffer_array) generic_array_destroy(eng->write_buffer_array);
  if (eng->buffer) free(eng->buffer);
  free(eng);
}

int engine_get_state(engine_t* engine)  {return engine->state;}

int engine_get_method(engine_t* engine)  {return engine->method;}

sample_count_t engine_get_buffer_size(engine_t* engine)  {return engine->buffer_size;}

int engine_initialize(engine_t* engine, int argc, const char **argv){
  int err, i, j, dont_start_engine;
  char* engine_method;
  struct poptOption optarray[15];
  poptContext optcontext;

  engine_method=0;
  dont_start_engine=0; /*TODO get rid of this or make it work*/

  /*sanity check, make sure engine hasn't been initialized yet*/
  if (engine_get_state(engine) != ENGINE_STATE_NOT_READY){
    printf("error: trying to initialize engine a second time\n");
    return -1;
  }

  /*parse command-line arguments using libpopt*/
  if (!(optarray[0].longName = strdup("method"))) {return -1;}
  optarray[0].shortName = 'm';
  optarray[0].argInfo = POPT_ARG_STRING;
  optarray[0].arg = (void*)&engine_method; 
  optarray[0].val = 1;
  optarray[0].descrip = "engine method";
  optarray[0].argDescrip = "either alsa or jack";
  if (!(optarray[1].longName = strdup("buffer"))) {return -1;}
  optarray[1].shortName = 'b';
  optarray[1].argInfo = POPT_ARG_INT;
  optarray[1].arg = (void*)&engine->buffer_size; 
  optarray[1].val = 2;
  optarray[1].descrip = "initial buffer size (alsa only)";
  optarray[1].argDescrip = "integer";
  if (!(optarray[2].longName = strdup("device"))) {return -1;}
  optarray[2].shortName = 'd';
  optarray[2].argInfo = POPT_ARG_STRING;
  optarray[2].arg = (void*)&engine->device_name; 
  optarray[2].val = 3;
  optarray[2].descrip = "alsa device name or jack client name (in both cases will use default if omitted)";
  optarray[2].argDescrip = "string";
  if (!(optarray[3].longName = strdup("rate"))) {return -1;}
  optarray[3].shortName = 'r';
  optarray[3].argInfo = POPT_ARG_LONG;
  optarray[3].arg = (void*)&engine->sample_rate; 
  optarray[3].val = 4;
  optarray[3].descrip = "sample rate (alsa only)";
  optarray[3].argDescrip = "integer";
  if (!(optarray[4].longName = strdup("channels"))) {return -1;}
  optarray[4].shortName = 'c';
  optarray[4].argInfo = POPT_ARG_INT;
  optarray[4].arg = (void*)&engine->num_channels; 
  optarray[4].val = 5;
  optarray[4].descrip = "number of channels (alsa only)";
  optarray[4].argDescrip = "integer";
  if (!(optarray[5].longName = strdup("interleaved"))) {return -1;}
  optarray[5].shortName = 'i';
  optarray[5].argInfo = POPT_ARG_NONE;
  optarray[5].arg = (void*)&engine->interleaved; 
  optarray[5].val = 5;
  optarray[5].descrip = "use interleaved output (alsa only)";
  optarray[5].argDescrip = "boolean";
  if (!(optarray[6].longName = strdup("periods"))) {return -1;}
  optarray[6].shortName = 'p';
  optarray[6].argInfo = POPT_ARG_INT;
  optarray[6].arg = (void*)&engine->num_periods; 
  optarray[6].val = 6;
  optarray[6].descrip = "number of periods (alsa only)";
  optarray[6].argDescrip = "integer";
  if (!(optarray[7].longName = strdup("periodsize"))) {return -1;}
  optarray[7].shortName = 's';
  optarray[7].argInfo = POPT_ARG_INT;
  optarray[7].arg = (void*)&engine->period_size; 
  optarray[7].val = 7;
  optarray[7].descrip = "size of periods (alsa only)";
  optarray[7].argDescrip = "integer";
  if (!(optarray[8].longName = strdup("availmin"))) {return -1;}
  optarray[8].shortName = 'a';
  optarray[8].argInfo = POPT_ARG_INT;
  optarray[8].arg = (void*)&engine->avail_min; 
  optarray[8].val = 7;
  optarray[8].descrip = "min # available frames required to generate sound card interrupt (alsa only)";
  optarray[8].argDescrip = "integer";
  if (!(optarray[9].longName = strdup("version"))) {return -1;}
  optarray[9].shortName = 'v';
  optarray[9].argInfo = POPT_ARG_NONE;
  optarray[9].arg = (void*)&debug_readout; 
  optarray[9].val = 7;
  optarray[9].descrip = "print Soundtank version and exit";
  optarray[9].argDescrip = "boolean";

  if (!(optarray[10].longName = strdup("no-engine"))) {return -1;}
  optarray[10].shortName = 'z';
  optarray[10].argInfo = POPT_ARG_NONE;
  optarray[10].arg = (void*)&dont_start_engine; 
  optarray[10].val = 7;
  optarray[10].descrip = "don't start engine";
  optarray[10].argDescrip = "boolean";
  optarray[11].longName = 0;
  optarray[11].shortName = '\0';
  optarray[11].argInfo = POPT_ARG_INCLUDE_TABLE;
  optarray[11].arg = poptHelpOptions; 
  optarray[11].val = 0;
  optarray[11].descrip = "Help options:";
  optarray[11].argDescrip = 0;
  optarray[12].longName =0;
  optarray[12].shortName = '\0';
  optarray[12].argInfo = 0;
  optarray[12].arg = 0; 
  optarray[12].val = 0;

  /*this does the actual argument parseing*/
  optcontext = poptGetContext("soundtank",argc,argv,optarray,0);
  j=0;
  while ((i=poptGetNextOpt(optcontext)) > 0){
    ++j;
    if ((i<0)&&(i!=-1)){
      printf("\n invalid option %s\n",argv[(j*2)+1]); /*TODO doesn't work*/
      return -1;
    }
  }
  poptFreeContext(optcontext);

  /*validate engine method*/
  if (!engine_method){
	  
    printf("no engine method selected, using alsa\n");
    engine->method = ENGINE_METHOD_ALSA;
      
  }else if (!strcmp(engine_method,"alsa")){
    
    engine->method = ENGINE_METHOD_ALSA;
    
  }else if (!strcmp(engine_method, "jack")){
    
    engine->method = ENGINE_METHOD_JACK;
    
  }else {
      
    printf("invalid argument for engine method\n");
    printf("valid choices are 'alsa' or 'jack'\n\n");
    return -1;
    
  }

  /*method specific initialization*/
  if (engine->method == ENGINE_METHOD_ALSA){
    
    if (((err = engine_alsa_initialize(engine)) < 0)) return err;
    
  }else if (engine->method == ENGINE_METHOD_JACK){
      
    if (((err = engine_jack_initialize(engine)) < 0)) return err;
    
  }

  return 0;
}



int engine_start(engine_t* engine){
  int ret;

  /*sanity check*/
  if (engine->state == ENGINE_STATE_ACTIVE) return 0;
  if (engine->state == ENGINE_STATE_NOT_READY) return -1;

  /*restart elapsed frames count*/
  engine->elapsed_frames = 0;

  /*call method dependent start function*/
  switch (engine->method){
  case ENGINE_METHOD_ALSA:
    
    if ((ret = engine_alsa_start(engine)) < 0){
      printf("error, couldn't start ALSA engine, error code %d\n", ret);
      return -1;
    }
    
    break;
  case ENGINE_METHOD_JACK:

    if ((ret = engine_jack_start(engine)) < 0){
      printf("error, couldn't start JACK engine\n");
      return -1;
    }

    break;
  default:
    printf("error, can't start engine because it is using an unknown method\n");
    return -1;
  }

  return 0;
}

int engine_stop(engine_t* engine){
  int ret;

  /*call method dependent stop function*/
  switch (engine->method){
  case ENGINE_METHOD_ALSA:
    
    if ((ret = engine_alsa_stop(engine)) < 0){
      printf("error, couldn't stop ALSA engine\n");
      return -1;
    }
    
    break;
  case ENGINE_METHOD_JACK:
    
    if ((ret = engine_jack_stop(engine)) < 0){
      printf("error, couldn't stop JACK engine\n");
      return -1;
    }

    break;
  default:
    printf("error, can't stop engine because it is using an unknown method\n");
    return -1;
  }

  return 0;
}

void engine_print_state(engine_t* engine){

  switch (engine->method){
  case ENGINE_METHOD_ALSA:
    engine_alsa_print_state();
    break;
  case ENGINE_METHOD_JACK:
    engine_jack_print_state(engine);
    break;
  default:
    printf("engine print state error: engine method is invalid\n");
  }

}

void engine_print_info(engine_t* engine, int argc, char** argv){

  if (engine->method == ENGINE_METHOD_ALSA)
    printf("method: ALSA\n");
  else if  (engine->method == ENGINE_METHOD_JACK)
    printf("method: JACK\n");
  else
    printf("method: NONE\n");

  printf("device name: %s\n",engine->device_name);
  printf("sample size: %d bits\n", engine->sample_size * 8);
  printf("buffer_size: %ld\n", engine->buffer_size);
  printf("sample rate: %u\n", engine->sample_rate);
  printf("num channels: %d\n", engine->num_channels);
  if (engine->interleaved == SND_PCM_ACCESS_RW_INTERLEAVED)
    printf("interleaved access\n");
  else
    printf("noninterleaved access\n");
  printf("num periods: %d\n", engine->num_periods);
  printf("period size: %ld\n", engine->period_size);
  printf("available min: %ld\n", engine->avail_min);
  printf("buffer address: %p\n", engine->buffer);

}

