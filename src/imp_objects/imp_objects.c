/*
 * implementation objects coordination code
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

#include <stdio.h>
#include <string.h>

#include "../include.h"



/*TODO: rename this validate imp type ... (oops) */
int validate_major_type(int major_type,int imp_type){

  switch (major_type){
  case RTOBJECT_MAJOR_TYPE_EXTERN_IN:
    if ((imp_type != RTOBJECT_IMP_TYPE_ALSA_EXTERN_IN)&&(imp_type != RTOBJECT_IMP_TYPE_JACK_EXTERN_IN)) return -1;
    break;    
  case RTOBJECT_MAJOR_TYPE_EXTERN_OUT:
    if ((imp_type != RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT)&&(imp_type != RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT)) return -1;
    break;
  case RTOBJECT_MAJOR_TYPE_SIGNAL_PATH:
    break;
  case RTOBJECT_MAJOR_TYPE_LOCAL_IN:
    if (imp_type != RTOBJECT_IMP_TYPE_LOCAL_IN) return -1;
    break;
  case RTOBJECT_MAJOR_TYPE_LOCAL_OUT:
    break;
  case RTOBJECT_MAJOR_TYPE_SOURCE:
    if ((imp_type != RTOBJECT_IMP_TYPE_AUDIO_FILE)&&(imp_type != RTOBJECT_IMP_TYPE_LADSPA_PLUGIN)&&(imp_type != RTOBJECT_IMP_TYPE_TEST_SOURCE)) return -1;
    break;
  case RTOBJECT_MAJOR_TYPE_FILTER:
    if (imp_type != RTOBJECT_IMP_TYPE_LADSPA_PLUGIN) return -1;
    break;
  case RTOBJECT_MAJOR_TYPE_CHANNEL_OPERATION:
    break;
  default:
    return -1; /*not a valid major type*/
  }

  return 0;
}


int create_rtobject_imp_object(rtobject_t* rtobj){
 
  switch (rtobj->imp_type){
  case RTOBJECT_IMP_TYPE_INLINE: 
    switch (rtobj->major_type){
    case RTOBJECT_MAJOR_TYPE_LOCAL_OUT:
      if ((create_imp_object_local_out(rtobj))<0) return -1;
      break;
    default:
      return -1;
      break;
    }
    break;    
  case RTOBJECT_IMP_TYPE_LOCAL_IN:
    if ((create_imp_object_local_in(rtobj))<0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_SIGNAL_PATH:
    if ((create_imp_object_signal_path(rtobj))<0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_ALSA_EXTERN_IN:
    /*TODO no ALSA extern in yet*/
    break;
  case RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT:      
    if (create_imp_object_alsa_extern_out(rtobj) < 0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_JACK_EXTERN_IN:
    if (create_imp_object_jack_extern_in(rtobj) < 0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT:      
    if (create_imp_object_jack_extern_out(rtobj) < 0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_LADSPA_PLUGIN:
    if (create_imp_object_ladspa_plugin(rtobj) < 0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_TEST_SOURCE:
    if (create_imp_object_test_source(rtobj) < 0) return -1;  
    break;
  case RTOBJECT_IMP_TYPE_CHANNEL_CP:
    if (create_imp_object_channel_cp(rtobj) < 0) return -1;
    break;
  default:
    return -1;
  }

  return 0;
}

int destroy_rtobject_imp_object(rtobject_t* rtobj){
  switch (rtobj->imp_type){
  case RTOBJECT_IMP_TYPE_INLINE: 
    switch (rtobj->major_type){
    case RTOBJECT_MAJOR_TYPE_LOCAL_OUT:
      if ((destroy_imp_object_local_out(rtobj))<0) return -1;
      break;
    default:
      return -1;
      break;
    }
    break;    
  case RTOBJECT_IMP_TYPE_LOCAL_IN:
    if ((destroy_imp_object_local_in(rtobj))<0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_SIGNAL_PATH:
    if ((destroy_imp_object_signal_path(rtobj))<0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_ALSA_EXTERN_IN:
    /*TODO no ALSA extern in yet*/
    break;
  case RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT:      
    if (destroy_imp_object_alsa_extern_out(rtobj) < 0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_JACK_EXTERN_IN:
    if (destroy_imp_object_jack_extern_in(rtobj) < 0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT:      
    if (destroy_imp_object_jack_extern_out(rtobj) < 0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_LADSPA_PLUGIN:
    if (destroy_imp_object_ladspa_plugin(rtobj) < 0) return -1;
    break;
  case RTOBJECT_IMP_TYPE_TEST_SOURCE:
    if (destroy_imp_object_test_source(rtobj) < 0) return -1;  
    break;
  case RTOBJECT_IMP_TYPE_CHANNEL_CP:
    if (destroy_imp_object_channel_cp(rtobj) < 0) return -1;
    break;
  default:
    return -1;
  }

  return 0; 
}


