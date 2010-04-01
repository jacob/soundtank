/*
 * Soundtank JACK (Jack Audio Connection Kit) engine code
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



#include <jack/jack.h>
 
#include "include.h"
#include "soundtank_structs.h"
	
static jack_client_t *client;


int soundtank_jack_process_callback(jack_nframes_t nframes, void *arg){
  soundtank_engine->elapsed_frames += nframes;
  return realtime_process_function(nframes);
}

void soundtank_jack_shutdown_callback(void *arg){

  soundtank_engine->state = ENGINE_STATE_INACTIVE;

  printf("\n ! audio processing has been stopped by JACK server !\n");

}

int engine_jack_initialize(engine_t* engine){

  if (!engine->device_name){
    if (!(engine->device_name = strdup("soundtank"))){
      printf("memory error allocating string\n");
      return -1;
    }
  }

  /*try to become a client of the JACK server*/  
  if ((client = jack_client_new(engine->device_name)) == 0){
    printf ("jack engine error: couldn't make jack client, jack server not running?\n");
    return -1;
  }

  /*get sample rate*/
  engine->sample_rate = jack_get_sample_rate(client);

  /*get period size (TODO: JACK does not guarantee a static period
    size however the current Soundtank code can't handle a change in
    the period size and will crash)*/
  engine->period_size = jack_get_buffer_size(client);

  /*set callbacks used by jack server*/
  if (jack_set_process_callback(client, soundtank_jack_process_callback, 0) < 0){
    printf("jack engine error: could not set processing callback function\n");
    return -1;
  }

  jack_on_shutdown(client, soundtank_jack_shutdown_callback, 0);

  /*engine is ready to go, set state variable to indicate so*/
  engine->state = ENGINE_STATE_INACTIVE;

  return 0;
}

int engine_jack_start(engine_t* engine){
  
  if (jack_activate(client)) 
    return -1;

  /*engine is now active*/
  engine->state = ENGINE_STATE_ACTIVE;

  return 0;
}

int engine_jack_stop(engine_t* engine){
  
  if (jack_deactivate(client)) 
    return -1;

  /*engine is now inactive*/
  engine->state = ENGINE_STATE_INACTIVE;

  return 0;
}

void engine_jack_print_state(engine_t* engine){

  switch (engine_get_state(engine)){
  case ENGINE_STATE_ACTIVE:
    printf("Active");
    break;
  case ENGINE_STATE_INACTIVE:
    printf("Not Active");
    break;
  default:
    printf("Invalid State");
    break;
  }

}

jack_client_t* engine_jack_get_client(){
  return client;
}
