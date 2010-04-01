/*
 * Soundtank - application data type functions
 *
 * Copyright Jacob Robbins 2003-2004
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


#include <alsa/asoundlib.h>
#include <ladspa.h>

#ifndef SOUNDTANK_TYPES_INCLUDE
#define SOUNDTANK_TYPES_INCLUDE


/*App Version is taken from autoconf-generated config.h*/
#define SOUNDTANK_APP_VERSION VERSION


/*Realtime Path Framework*/

#define RTOBJECT_MAJOR_TYPE_SIGNAL_PATH 1
#define RTOBJECT_MAJOR_TYPE_EXTERN_IN 2
#define RTOBJECT_MAJOR_TYPE_EXTERN_OUT 3
#define RTOBJECT_MAJOR_TYPE_LOCAL_IN 4
#define RTOBJECT_MAJOR_TYPE_LOCAL_OUT 5
#define RTOBJECT_MAJOR_TYPE_SOURCE 6
#define RTOBJECT_MAJOR_TYPE_FILTER 7
#define RTOBJECT_MAJOR_TYPE_CHANNEL_OPERATION 8

int string_to_rtobject_major_type(const char* input_string);
char* rtobject_major_type_to_string(int type);


#define RTOBJECT_IMP_TYPE_INLINE 0
#define RTOBJECT_IMP_TYPE_SIGNAL_PATH 1
#define RTOBJECT_IMP_TYPE_ALSA_EXTERN_IN 2
#define RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT 3
#define RTOBJECT_IMP_TYPE_JACK_EXTERN_IN 4
#define RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT 5
#define RTOBJECT_IMP_TYPE_LOCAL_IN 6
#define RTOBJECT_IMP_TYPE_TEST_SOURCE 7
#define RTOBJECT_IMP_TYPE_AUDIO_FILE 8
#define RTOBJECT_IMP_TYPE_LADSPA_PLUGIN 9
#define RTOBJECT_IMP_TYPE_CHANNEL_CP 10


int string_to_rtobject_imp_type(const char* input_string);
char* rtobject_imp_type_to_string(int imp_type);
int major_type_to_default_imp_type(int major_type);



/*IPC Messaging*/

#define MSG_SIZE 4

#define MSG_TYPE_SWAP_ADDRESS_LIST 1
#define MSG_TYPE_FINISHED_ADDRESS_LIST 12
#define MSG_TYPE_CHANGE_BUFFER_SIZE 3
#define MSG_TYPE_FINISHED_BUFFER_SIZE 14
#define MSG_TYPE_SWAP_PROCESS_LISTS 5
#define MSG_TYPE_FINISHED_PROCESS_LISTS 16
#define MSG_TYPE_SWAP_GENERIC_POINTER 7
#define MSG_TYPE_FINISHED_GENERIC_POINTER 18
#define MSG_TYPE_STOP_ENGINE 9
#define MSG_TYPE_FINISHED_STOP_ENGINE 20

struct swap_message{
  long int msg_type;
  char msg_content[MSG_SIZE];
};


/*Engine Errors*/

#define RT_ERROR_READOUT_LOUD 0
#define RT_ERROR_READOUT_QUIET 1
#define RT_ERROR_READOUT_OFF 2

#define SOUNDTANK_RT_ERROR 3



/*ALSA event stuff*/

/*kind of weird that ALSA doesn't give us something to talk about
  event parameter fields with like...*/
#define EVENT_PARAM_CHANNEL 0
#define EVENT_PARAM_NOTE 1
#define EVENT_PARAM_VELOCITY 2
#define EVENT_PARAM_OFFVELOCITY 3
#define EVENT_PARAM_DURATION 4
#define EVENT_PARAM_CCPARAM 5
#define EVENT_PARAM_CCVALUE 6
#define EVENT_PARAM_NONE 7




/*Controls*/

#define CONTROL_DESC_ACTIVE 1
#define CONTROL_DESC_MUTE 2
#define CONTROL_DESC_VOLUME 3
#define CONTROL_DESC_PITCH 4
#define CONTROL_DESC_WETNESS 5

#define CONTROL_DESC_STRING_ACTIVE "Active"
#define CONTROL_DESC_STRING_MUTE "Mute"
#define CONTROL_DESC_STRING_VOLUME "Volume"
#define CONTROL_DESC_STRING_PITCH "Pitch"
#define CONTROL_DESC_STRING_WETNESS "Wetness"


/*Data Ports*/

#define DESCRIPTION_FAMILY_LIBCOUSIN 1
#define DESCRIPTION_FAMILY_LADSPA 2

enum libcousin_data_port_description{
  mono,
  stereo_l,
  stereo_r
};

#define DATA_PORT_DESCRIPTION_STRING_MONO "mono"
#define DATA_PORT_DESCRIPTION_STRING_STEREO_L "stereo_l"
#define DATA_PORT_DESCRIPTION_STRING_STEREO_R "stereo_r"

#define BUFFER_TYPE_MONO 0
typedef struct mono_audio_buffer_instance buffer_instance_t;
typedef struct mono_audio_buffer buffer_t;


typedef snd_pcm_sframes_t sample_count_t;  /*note: alsa dependency*/
typedef float sample_t;





#endif


