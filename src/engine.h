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

#ifndef SOUNDTANK_ENGINE_INCLUDE
#define SOUNDTANK_ENGINE_INCLUDE

#include <jack/jack.h>

#define ENGINE_METHOD_NONE 0
#define ENGINE_METHOD_ALSA 1
#define ENGINE_METHOD_JACK 2

#define ENGINE_STATE_NOT_READY 0
#define ENGINE_STATE_INACTIVE 1
#define ENGINE_STATE_ACTIVE 2


typedef struct pair_i_v pair_i_v_t;
struct pair_i_v{ int owner_object_address; void* data;};

typedef struct soundtank_engine_struct engine_t;
struct soundtank_engine_struct{

  int state;
  int method;
  char* device_name;
  size_t sample_size;
  sample_count_t buffer_size;
  unsigned int sample_rate; 
  unsigned int num_channels;
  int interleaved;
  int num_periods; 
  sample_count_t period_size;
  unsigned long avail_min;

  /*stats*/
  long long elapsed_frames;
  long long last_xrun;
  long long total_xruns;

  /*buffer(s) used to send audio data to ALSA*/
  /*JACK buffers are accessed through JACK api*/
  generic_array_t* write_buffer_array;
  void* buffer;

};



engine_t* engine_alloca();
void engine_dealloca(engine_t* eng);

int engine_get_state(engine_t* engine);
int engine_get_method(engine_t* engine);
sample_count_t engine_get_buffer_size(engine_t* engine);

int engine_initialize(engine_t* engine, int argc, const char** argv);

int engine_start(engine_t* engine);
int engine_stop(engine_t* engine);

void engine_print_state(engine_t* engine);
void engine_print_info(engine_t* engine, int argc, char** argv);

/*ALSA engine functions, from engine_alsa.c*/
int engine_alsa_initialize(engine_t* engine);
int engine_alsa_initialize_buffer(engine_t* engine);

void* engine_alsa_attach_to_device_output_channel(engine_t* engine,\
					     int channel,\
					     int object_address);
void engine_alsa_detach_from_device_output_channel(engine_t* engine,\
					      int channel,\
					      int object_address);

int engine_alsa_start(engine_t* engine);
int engine_alsa_stop(engine_t* engine);

void* engine_alsa_poll_loop(void * eng_ptr);
void engine_alsa_print_state();

/*JACK engine functions, from engine_jack.c*/
int engine_jack_initialize(engine_t* engine);

int engine_jack_start(engine_t* engine);
int engine_jack_stop(engine_t* engine);

void engine_jack_print_state(engine_t* engine);

jack_client_t* engine_jack_get_client();

#endif
