/*
 * rtobject instance code
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

#include "include.h"

rtobject_instance_t* rtobject_instance_alloca(){
  rtobject_instance_t* ins;

  if (!(ins = (rtobject_instance_t*)malloc(sizeof(rtobject_instance_t)))){
    return 0;
  }

  ins->imp_type = -1;
  ins->active_before = 0;
  ins->unmuted_before = 0;
  ins->data_port_list_size = 0;
  ins->data_port_list = 0;
  ins->control_list_size = 0;
  ins->control_list = 0;
  ins->note_flag = 0;
  ins->process_index = -1;
  ins->major_type = -1;

  memset(&(ins->imp_data), 0, sizeof(imp_data_t));

  return ins;
}

rtobject_instance_t* rtobject_instance_alloca_copy(const rtobject_instance_t* orig_ins){
  /*NOTE: The point of this copy allocator is to make sure that
    duplicated instances do not have access to _any_ of the instance
    specific memory used by the instances they're copied from.
    Instances are duplicated when changes are made to their data_port
    connections*/
  rtobject_instance_t* ins;
  int i;

  if (!(ins = rtobject_instance_alloca())){
    return 0;
  }

  ins->imp_type = orig_ins->imp_type;
  ins->active_before = orig_ins->active_before;
  ins->unmuted_before = orig_ins->unmuted_before;
  ins->note_flag = orig_ins->note_flag;
  ins->process_index = orig_ins->process_index;
  ins->major_type = orig_ins->major_type;

  /*copy data port list values*/
  ins->data_port_list_size = orig_ins->data_port_list_size;
  if (ins->data_port_list_size){
    if (!(ins->data_port_list = (data_port_instance_t*)\
	  malloc(ins->data_port_list_size * sizeof(data_port_instance_t)))){
      rtobject_instance_dealloca(ins);
      return 0;
    }
  }
  for (i=0; i < ins->data_port_list_size; ++i)
    ins->data_port_list[i] = orig_ins->data_port_list[i];

  /*copy control list values*/
  ins->control_list_size = orig_ins->control_list_size;
  if (ins->control_list_size){
    if (!(ins->control_list = (control_instance_t*)\
	  malloc(ins->control_list_size * sizeof(control_instance_t)))){
      rtobject_instance_dealloca(ins);
      return 0;
    }
  }
  for (i=0; i < ins->control_list_size; ++i)
    ins->control_list[i] = orig_ins->control_list[i];

  /*SHOULD WE DO THIS? initialize the imp_data union here*/
  switch (ins->imp_type){
    
  case RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT:

    ins->imp_data.extern_out_element.device_channel = \
      orig_ins->imp_data.extern_out_element.device_channel;
    ins->imp_data.extern_out_element.device_buffer = \
      orig_ins->imp_data.extern_out_element.device_buffer;
    break;

  case RTOBJECT_IMP_TYPE_JACK_EXTERN_IN:
    ins->imp_data.jack_extern_element.jack_port = \
      orig_ins->imp_data.jack_extern_element.jack_port;
    break;

  case RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT:
    ins->imp_data.jack_extern_element.jack_port = \
      orig_ins->imp_data.jack_extern_element.jack_port;
    break;

  case RTOBJECT_IMP_TYPE_SIGNAL_PATH:
    {
      node_t* temp_node;
      for (temp_node = orig_ins->imp_data.signal_path_element.local_buffer_list; \
	     temp_node;
	   temp_node = temp_node->next){
	
	if (!(ll_append(&ins->imp_data.signal_path_element.local_buffer_list, \
			temp_node->data))){
	  rtobject_instance_dealloca(ins);
	  return 0;
	}
      }
    }
    break;
    
  case RTOBJECT_IMP_TYPE_LADSPA_PLUGIN:
    { 
      node_t* temp_node;

      ins->imp_data.ladspa_plugin_element.descriptor = \
	orig_ins->imp_data.ladspa_plugin_element.descriptor;
      ins->imp_data.ladspa_plugin_element.overwrite = \
	orig_ins->imp_data.ladspa_plugin_element.overwrite;
      ins->imp_data.ladspa_plugin_element.handle = \
	orig_ins->imp_data.ladspa_plugin_element.handle;    

      for (temp_node = orig_ins->imp_data.ladspa_plugin_element.output_data_port_list; \
	     temp_node;
	   temp_node = temp_node->next){
	
	if (!(ll_append(&ins->imp_data.ladspa_plugin_element.output_data_port_list, \
			temp_node->data))){
	  rtobject_instance_dealloca(ins);
	  return 0;
	}

      }

    } 
    break;

  default:
    break;
  }

  return ins;
}

void rtobject_instance_dealloca(rtobject_instance_t* ins){

  if (ins->data_port_list) free(ins->data_port_list); 
  if (ins->control_list) free(ins->control_list);

  if (ins->imp_type == RTOBJECT_IMP_TYPE_SIGNAL_PATH){

    while (ins->imp_data.signal_path_element.local_buffer_list){

	ll_remove(ins->imp_data.signal_path_element.local_buffer_list,\
		  &ins->imp_data.signal_path_element.local_buffer_list);

      }
 
  }

  free(ins); 

}

int create_rtobject_instance(rtobject_t* rtobj){
  rtobject_instance_t* ins;
  int i;

  /*TODO: make sure rtobject is of type allowed more than one instance*/

  /*copy an existing instance if there is one*/
  if (rtobject_get_instance_list_size(rtobj) > 0){
    rtobject_instance_t* orig_ins;

    /*copy last instance*/
    if (!(orig_ins = \
	  rtobject_get_instance(rtobj, rtobject_get_instance_list_size(rtobj) - 1))){
      printf("create instance error: couldn't access previous instance\n");
      return -1;
    }

    if (!(ins = rtobject_instance_alloca_copy(orig_ins))){
      printf("create instance error: failed to copy previous instance\n");
      return -1;
    }

  }else{
    /*if no instances exist yet we must make one & initialize its
      ports & controls using the rtobject's ports & controls*/

    /*allocate new instance*/
    if (!(ins = rtobject_instance_alloca())){
      return -1;
    }

    /*set instance imp type*/
    ins->imp_type = rtobject_get_implementation_type(rtobj);
    ins->major_type = rtobject_get_major_type(rtobj);
    
    /*allocate data ports*/
    ins->data_port_list_size = rtobject_get_data_port_list_size(rtobj);
    if (ins->data_port_list_size){
      if (!(ins->data_port_list = (data_port_instance_t*)\
	    malloc(ins->data_port_list_size * sizeof(data_port_instance_t)))){
	rtobject_instance_dealloca(ins);
	return -1;
      }
    }

    /*initialize data ports*/
    for (i=0; i < ins->data_port_list_size; ++i){
      data_port_t* curr_port;
      
      /*get rtobject's data port*/
      if (!(curr_port = rtobject_get_data_port(rtobj,i))){
	printf("create instance error: failure access data port %d\n", i);
	rtobject_instance_dealloca(ins);
	return -1;
      }

      ins->data_port_list[i] = data_port_get_buffer_struct(curr_port);

    }

    /*allocate controls*/
    ins->control_list_size = rtobject_get_control_list_size(rtobj);
    if (ins->control_list_size){
      if (!(ins->control_list = (control_instance_t*)\
	    malloc(ins->control_list_size * sizeof(control_instance_t)))){
	rtobject_instance_dealloca(ins);
	return -1;
      }
    }
    
    /*initialize controls*/
    for (i=0; i < ins->control_list_size; ++i){

      /*get rtobject's control*/
      const control_t* curr_control = rtobject_get_control(rtobj, i);
      
      /*sanity check*/
      if (!curr_control){
	printf("create instance error: can not access rtobject control %d\n", i);
	rtobject_instance_dealloca(ins);
	return -1;
      }

      /*set controls to default values if they exist*/
      if (control_get_range_flags(curr_control) & RANGE_DEFAULT_EXISTS)
	ins->control_list[i] = control_get_range_def(curr_control);
      else
	ins->control_list[i] = 0;
           
    }

  }

  /*append new instance to object's instance list*/
  if (!(ll_append(&rtobj->instance_list,(void*)ins))){
   rtobject_instance_dealloca(ins);
   return -1;
  }

  rtobj->instance_list_size++;

  /*initialize instance imp_data using rtobject's implementation object*/
  switch (ins->major_type){
    
  case RTOBJECT_MAJOR_TYPE_EXTERN_IN:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_JACK_EXTERN_IN){
      if ((init_instance_jack_extern_in(rtobj, ins)) < 0) return -1;
    }      
    break;
  case RTOBJECT_MAJOR_TYPE_EXTERN_OUT:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT){
      if ((init_instance_alsa_extern_out(rtobj, ins)) < 0) return -1;
    }
    if (ins->imp_type == RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT){
      if ((init_instance_jack_extern_out(rtobj, ins)) < 0) return -1;
    }      
    break;
  case RTOBJECT_MAJOR_TYPE_SIGNAL_PATH:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_SIGNAL_PATH){
      if ((init_instance_signal_path(rtobj, ins)) < 0) return -1;
    }
    break;
  case RTOBJECT_MAJOR_TYPE_LOCAL_IN:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_INLINE){
      if ((init_instance_local_in(rtobj, ins)) < 0) return -1;
    }  
    break;
  case RTOBJECT_MAJOR_TYPE_LOCAL_OUT:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_INLINE){
      if ((init_instance_local_out(rtobj, ins)) < 0) return -1;
    }  
    break;
  case RTOBJECT_MAJOR_TYPE_SOURCE:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_AUDIO_FILE){

    }else if (ins->imp_type == RTOBJECT_IMP_TYPE_LADSPA_PLUGIN){
      if ((init_instance_ladspa_plugin(rtobj, ins)) < 0) return -1;   

    }else if (ins->imp_type == RTOBJECT_IMP_TYPE_TEST_SOURCE){
      if ((init_instance_test_source(rtobj, ins)) < 0) return -1;   
    }
    break;
  case RTOBJECT_MAJOR_TYPE_FILTER:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_LADSPA_PLUGIN){
      if ((init_instance_ladspa_plugin(rtobj, ins)) < 0) return -1;  
    }
    break;
  case RTOBJECT_MAJOR_TYPE_CHANNEL_OPERATION:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_CHANNEL_CP){
      if ((init_instance_channel_cp(rtobj, ins)) < 0) return -1;  
    }
    break;
  default:
    return -1; /*not a valid major type*/
  }

  /*NOTE: we don't call rtobject update instance situation here
    because it is up to caller to do that*/
   
  return 0;
}

int create_rtobject_instance_copy(rtobject_t* rtobj, rtobject_instance_t* orig_ins){
  rtobject_instance_t* ins;

  /*TODO: make sure rtobject is of type allowed more than one instance*/

  /*copy instance*/
  if (!(ins = rtobject_instance_alloca_copy(orig_ins))){
    printf("create instance copy error: failed to copy orig instance\n");
    return -1;
  }

  /*append new instance to object's instance list*/
  if (!(ll_append(&rtobj->instance_list,(void*)ins))){
   rtobject_instance_dealloca(ins);
   return -1;
  }

  rtobj->instance_list_size++;

  /*initialize instance imp_data using rtobject's implementation object*/
  switch (ins->major_type){
    
  case RTOBJECT_MAJOR_TYPE_EXTERN_IN:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_JACK_EXTERN_IN){
      if ((init_instance_jack_extern_in(rtobj, ins)) < 0) return -1;
    }      
    break;
  case RTOBJECT_MAJOR_TYPE_EXTERN_OUT:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT){
      if ((init_instance_alsa_extern_out(rtobj, ins)) < 0) return -1;
    }
    if (ins->imp_type == RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT){
      if ((init_instance_jack_extern_out(rtobj, ins)) < 0) return -1;
    }      
    break;
  case RTOBJECT_MAJOR_TYPE_SIGNAL_PATH:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_SIGNAL_PATH){
      if ((init_instance_signal_path(rtobj, ins)) < 0) return -1;
    }
    break;
  case RTOBJECT_MAJOR_TYPE_LOCAL_IN:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_INLINE){
      if ((init_instance_local_in(rtobj, ins)) < 0) return -1;
    }  
    break;
  case RTOBJECT_MAJOR_TYPE_LOCAL_OUT:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_INLINE){
      if ((init_instance_local_out(rtobj, ins)) < 0) return -1;
    }  
    break;
  case RTOBJECT_MAJOR_TYPE_SOURCE:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_AUDIO_FILE){

    }else if (ins->imp_type == RTOBJECT_IMP_TYPE_LADSPA_PLUGIN){
      if ((init_instance_ladspa_plugin(rtobj, ins)) < 0) return -1;   

    }else if (ins->imp_type == RTOBJECT_IMP_TYPE_TEST_SOURCE){
      if ((init_instance_test_source(rtobj, ins)) < 0) return -1;   
    }
    break;
  case RTOBJECT_MAJOR_TYPE_FILTER:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_LADSPA_PLUGIN){
      if ((init_instance_ladspa_plugin(rtobj, ins)) < 0) return -1;  
    }
    break;
  case RTOBJECT_MAJOR_TYPE_CHANNEL_OPERATION:
    if (ins->imp_type == RTOBJECT_IMP_TYPE_CHANNEL_CP){
      if ((init_instance_channel_cp(rtobj, ins)) < 0) return -1;  
    }
    break;
  default:
    return -1; /*not a valid major type*/
  }

  /*NOTE: we don't call rtobject update instance situation here
    because it is up to caller to do that*/
   
  return 0;
}

int destroy_rtobject_instance(rtobject_t* rtobj){
  rtobject_instance_t* old_instance;

  if (!rtobj->instance_list){
    /*no more instances to remove*/
    return 0;
  }

  /*we remove first instance cause it's easier*/
  old_instance = (rtobject_instance_t*)rtobj->instance_list->data;

  /*remove instance from rtobject's instance list*/
  ll_remove(rtobj->instance_list,&rtobj->instance_list);
  rtobj->instance_list_size--;

  /*update rtobject's "instance situation"*/
  if ((rtobject_update_instance_situation(rtobj)) < 0){
    printf("free instance error: could not update rtobject w/o instance\n");
    return -1;
  }

  /*TODO: fix this quick hack, it calls remake-proclist too many times*/
  /*       maybe switch to a set-instance-count(int cnt) version */
  if ((remake_process_list()) < 0){
    printf("free instance error: unable to update proclist\n");
    return -1;
  }
  
  /*deinitialize instance in implementation object*/
  switch (old_instance->major_type){
    
  case RTOBJECT_MAJOR_TYPE_EXTERN_IN:
    if (old_instance->imp_type == RTOBJECT_IMP_TYPE_JACK_EXTERN_IN){
      if ((deinit_instance_jack_extern_in(rtobj, old_instance)) < 0) return -1;
    }      
    break;
  case RTOBJECT_MAJOR_TYPE_EXTERN_OUT:
    if (old_instance->imp_type == RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT){
      if ((deinit_instance_alsa_extern_out(rtobj, old_instance)) < 0) return -1;
    }
    if (old_instance->imp_type == RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT){
      if ((deinit_instance_jack_extern_out(rtobj, old_instance)) < 0) return -1;
    }      
    break;
  case RTOBJECT_MAJOR_TYPE_SIGNAL_PATH:
    if (old_instance->imp_type == RTOBJECT_IMP_TYPE_SIGNAL_PATH){
      if ((deinit_instance_signal_path(rtobj, old_instance)) < 0) return -1;
    }
    break;
  case RTOBJECT_MAJOR_TYPE_LOCAL_IN:
    if (old_instance->imp_type == RTOBJECT_IMP_TYPE_INLINE){
      if ((deinit_instance_local_in(rtobj, old_instance)) < 0) return -1;
    }  
    break;
  case RTOBJECT_MAJOR_TYPE_LOCAL_OUT:
    if (old_instance->imp_type == RTOBJECT_IMP_TYPE_INLINE){
      if ((deinit_instance_local_out(rtobj, old_instance)) < 0) return -1;
    }  
    break;
  case RTOBJECT_MAJOR_TYPE_SOURCE:
    if (old_instance->imp_type == RTOBJECT_IMP_TYPE_AUDIO_FILE){

    }else if (old_instance->imp_type == RTOBJECT_IMP_TYPE_LADSPA_PLUGIN){
      if ((deinit_instance_ladspa_plugin(rtobj, old_instance)) < 0) return -1;   

    }else if (old_instance->imp_type == RTOBJECT_IMP_TYPE_TEST_SOURCE){
      if ((deinit_instance_test_source(rtobj, old_instance)) < 0) return -1;   
    }
    break;
  case RTOBJECT_MAJOR_TYPE_FILTER:
    if (old_instance->imp_type == RTOBJECT_IMP_TYPE_LADSPA_PLUGIN){
	if ((deinit_instance_ladspa_plugin(rtobj, old_instance)) < 0) return -1;  
    }
    break;
  case RTOBJECT_MAJOR_TYPE_CHANNEL_OPERATION:
    if (old_instance->imp_type == RTOBJECT_IMP_TYPE_CHANNEL_CP){
      if ((deinit_instance_channel_cp(rtobj, old_instance)) < 0) return -1;  
    }
    break;
  default:
    return -1; /*not a valid major type*/
  }

  /*dealloca removed instance*/
  rtobject_instance_dealloca(old_instance);

  return 0;
}





int rtobject_instance_attach_data_ports(rtobject_t* rtobj, rtobject_instance_t* rtins){
  /*set rtinstance buffers to match rtobject's*/
  data_port_t* curr_port;
  int i;

  /*TODO: verify instance belongs to object*/

  for (i=0;i<rtobj->data_port_list_size;++i){
    curr_port = rtobject_get_data_port(rtobj,i);
    rtins->data_port_list[i] = data_port_get_buffer_struct(curr_port);
  }

  if (RTOBJECT_IMP_TYPE_LADSPA_PLUGIN == rtobject_get_implementation_type(rtobj))
    attach_instance_ladspa_plugin(rtobj, rtins);


  return 0;
}

int rtobject_instance_detach_data_ports(rtobject_t* rtobj, rtobject_instance_t* rtins){
  /*set rtinstance buffers 0*/
  int i;

  for (i=0;i<rtobj->data_port_list_size;++i)
    rtins->data_port_list[i] = 0;

  if (RTOBJECT_IMP_TYPE_LADSPA_PLUGIN == rtobject_get_implementation_type(rtobj))
    detach_instance_ladspa_plugin(rtobj, rtins);

  return 0;
}

