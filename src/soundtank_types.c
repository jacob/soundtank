/*
 * Soundtank - application data type functions
 *
 * Copyright Jacob Robbins 2004
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
#include <string.h>

#include "include.h"
#include "soundtank_structs.h"

int string_to_rtobject_major_type(const char* input_string){

  if (!(strcmp(input_string,"extern_in"))) return RTOBJECT_MAJOR_TYPE_EXTERN_IN;
  if (!(strcmp(input_string,"extern_out"))) return RTOBJECT_MAJOR_TYPE_EXTERN_OUT;
  if (!(strcmp(input_string,"ex_in"))) return RTOBJECT_MAJOR_TYPE_EXTERN_IN;
  if (!(strcmp(input_string,"ex_out"))) return RTOBJECT_MAJOR_TYPE_EXTERN_OUT;
  if (!(strcmp(input_string,"signal_path"))) return RTOBJECT_MAJOR_TYPE_SIGNAL_PATH;
  if (!(strcmp(input_string,"sigpath"))) return RTOBJECT_MAJOR_TYPE_SIGNAL_PATH;
  if (!(strcmp(input_string,"path"))) return RTOBJECT_MAJOR_TYPE_SIGNAL_PATH;
  if (!(strcmp(input_string,"local_in"))) return RTOBJECT_MAJOR_TYPE_LOCAL_IN;
  if (!(strcmp(input_string,"in"))) return RTOBJECT_MAJOR_TYPE_LOCAL_IN;
  if (!(strcmp(input_string,"local_out"))) return RTOBJECT_MAJOR_TYPE_LOCAL_OUT;
  if (!(strcmp(input_string,"out"))) return RTOBJECT_MAJOR_TYPE_LOCAL_OUT;
  if (!(strcmp(input_string,"source"))) return RTOBJECT_MAJOR_TYPE_SOURCE;
  if (!(strcmp(input_string,"src"))) return RTOBJECT_MAJOR_TYPE_SOURCE;
  if (!(strcmp(input_string,"filter"))) return RTOBJECT_MAJOR_TYPE_FILTER;
  if (!(strcmp(input_string,"flt"))) return RTOBJECT_MAJOR_TYPE_FILTER;
  if (!(strcmp(input_string,"channel_op"))) return RTOBJECT_MAJOR_TYPE_CHANNEL_OPERATION;
  if (!(strcmp(input_string,"op"))) return RTOBJECT_MAJOR_TYPE_CHANNEL_OPERATION;
  return -1;
}

char* rtobject_major_type_to_string(int type){

  switch (type){
  case RTOBJECT_MAJOR_TYPE_EXTERN_IN:
    return strdup("extern_in");
  case RTOBJECT_MAJOR_TYPE_EXTERN_OUT:
    return strdup("extern_out");    
  case RTOBJECT_MAJOR_TYPE_SIGNAL_PATH:
    return strdup("path");
  case RTOBJECT_MAJOR_TYPE_LOCAL_IN:
    return strdup("local_in");
  case RTOBJECT_MAJOR_TYPE_LOCAL_OUT:
    return strdup("local_out");
  case RTOBJECT_MAJOR_TYPE_SOURCE:
    return strdup("src");
  case RTOBJECT_MAJOR_TYPE_FILTER:
    return strdup("filter");
  case RTOBJECT_MAJOR_TYPE_CHANNEL_OPERATION:
    return strdup("channel_op");
  }

  return 0;
}

int string_to_rtobject_imp_type(const char* input_string){

  if (!(strcmp(input_string,"builtin"))) return RTOBJECT_IMP_TYPE_INLINE;
  if (!(strcmp(input_string,"inline"))) return RTOBJECT_IMP_TYPE_INLINE;
  if (!(strcmp(input_string,"alsa_extern_in"))) return RTOBJECT_IMP_TYPE_ALSA_EXTERN_IN;
  if (!(strcmp(input_string,"alsa_extern_out"))) return RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT;
  if (!(strcmp(input_string,"jack_extern_in"))) return RTOBJECT_IMP_TYPE_JACK_EXTERN_IN;
  if (!(strcmp(input_string,"jack_extern_out"))) return RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT;
  if (!(strcmp(input_string,"local_in"))) return RTOBJECT_IMP_TYPE_LOCAL_IN;
  if (!(strcmp(input_string,"signal_path"))) return RTOBJECT_IMP_TYPE_SIGNAL_PATH;
  if (!(strcmp(input_string,"path"))) return RTOBJECT_IMP_TYPE_SIGNAL_PATH;
  if (!(strcmp(input_string,"test"))) return RTOBJECT_IMP_TYPE_TEST_SOURCE;
  if (!(strcmp(input_string,"file"))) return RTOBJECT_IMP_TYPE_AUDIO_FILE;
  if (!(strcmp(input_string,"ladspa_plugin"))) return RTOBJECT_IMP_TYPE_LADSPA_PLUGIN;
  if (!(strcmp(input_string,"ladspa"))) return RTOBJECT_IMP_TYPE_LADSPA_PLUGIN;
  if (!(strcmp(input_string,"channel_cp"))) return RTOBJECT_IMP_TYPE_CHANNEL_CP;
  if (!(strcmp(input_string,"chan_cp"))) return RTOBJECT_IMP_TYPE_CHANNEL_CP;
  if (!(strcmp(input_string,"cp"))) return RTOBJECT_IMP_TYPE_CHANNEL_CP;
  return -1;
}

char* rtobject_imp_type_to_string(int imp_type){

  switch (imp_type){
  case RTOBJECT_IMP_TYPE_INLINE:
    return strdup("inline");
  case RTOBJECT_IMP_TYPE_ALSA_EXTERN_IN:
    return strdup("alsa_extern_in");
  case RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT:
    return strdup("alsa_extern_out");
  case RTOBJECT_IMP_TYPE_LOCAL_IN:
    return strdup("local_in");
  case RTOBJECT_IMP_TYPE_JACK_EXTERN_IN:
    return strdup("jack_extern_in");
  case RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT:
    return strdup("jack_extern_out");
  case RTOBJECT_IMP_TYPE_SIGNAL_PATH:
    return strdup("path");
  case RTOBJECT_IMP_TYPE_TEST_SOURCE:
    return strdup("test");
  case RTOBJECT_IMP_TYPE_AUDIO_FILE:
    return strdup("file");
  case RTOBJECT_IMP_TYPE_LADSPA_PLUGIN:
    return strdup("ladspa");
  case RTOBJECT_IMP_TYPE_CHANNEL_CP:
    return strdup("cp");
  }

  return 0;
}

int major_type_to_default_imp_type(int major_type){

  /*TODO FILL THIS UP MORE*/
  if (major_type ==  RTOBJECT_MAJOR_TYPE_SIGNAL_PATH ) return RTOBJECT_IMP_TYPE_SIGNAL_PATH;
  if (major_type ==  RTOBJECT_MAJOR_TYPE_SOURCE ) return RTOBJECT_IMP_TYPE_LADSPA_PLUGIN;
  if (major_type ==  RTOBJECT_MAJOR_TYPE_FILTER ) return RTOBJECT_IMP_TYPE_LADSPA_PLUGIN;
  if (major_type ==  RTOBJECT_MAJOR_TYPE_LOCAL_IN ) return RTOBJECT_IMP_TYPE_LOCAL_IN;
  if (major_type ==  RTOBJECT_MAJOR_TYPE_EXTERN_IN ){
    if (engine_get_method(soundtank_engine) == ENGINE_METHOD_ALSA)
      return RTOBJECT_IMP_TYPE_ALSA_EXTERN_IN;
    if (engine_get_method(soundtank_engine) == ENGINE_METHOD_JACK)
      return RTOBJECT_IMP_TYPE_JACK_EXTERN_IN;
    return -1;
  }
  if (major_type ==  RTOBJECT_MAJOR_TYPE_EXTERN_OUT ){
    if (engine_get_method(soundtank_engine) == ENGINE_METHOD_ALSA)
      return RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT;
    if (engine_get_method(soundtank_engine) == ENGINE_METHOD_JACK)
      return RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT;
    return -1;
  }
  return  RTOBJECT_IMP_TYPE_INLINE;
}

