/*
 * Soundtank realtime process function code
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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include <jack/jack.h>

#include "include.h"
#include "soundtank_structs.h"


struct swap_message local_msg;

int realtime_process_function(sample_count_t numframes){
  int curr,i;
  node_t* temp_node;
  snd_seq_event_t* curr_event;
  register rtobject_instance_t* curr_instance;

  /*check for messages (nonblocking)*/
  if ((msgrcv(msgid,&local_msg,(size_t)MSG_SIZE,-10, IPC_NOWAIT)) > 0){

    if (debug_readout){
      printf("\nRTThread Got a Message \n");
      printf("message type = %ld\n",local_msg.msg_type);
    }
    

    /*handle messages*/
    switch  (local_msg.msg_type) {
      
    case MSG_TYPE_SWAP_ADDRESS_LIST:{
      int temp_size;
      rtobject_t** temp_swap;
      temp_swap = rtobject_address_list;
      temp_size = rtobject_address_list_size;
      rtobject_address_list = swap_rtobject_address_list;
      rtobject_address_list_size = swap_rtobject_address_list_size;
      swap_rtobject_address_list = temp_swap;
      swap_rtobject_address_list_size = temp_size;
      local_msg.msg_type = MSG_TYPE_FINISHED_ADDRESS_LIST;
      if (msgsnd(msgid,&local_msg,(size_t)MSG_SIZE,0)) return -1; /*TODO handle failure*/
      break;
    }

    case MSG_TYPE_CHANGE_BUFFER_SIZE:{
      
      break;
    }

    case MSG_TYPE_SWAP_PROCESS_LISTS:{
      rtobject_instance_t** temp_ptr;
      if (swap_process_list_size){
	temp_ptr = process_list;
	curr = process_list_size;
	process_list = swap_process_list;
	process_list_size = swap_process_list_size;
	/*hack to handle emptying the list*/
	if (process_list_size < 0) process_list_size = 0;
	swap_process_list = temp_ptr;
	swap_process_list_size = curr;
      }

      local_msg.msg_type = MSG_TYPE_FINISHED_PROCESS_LISTS;
      if (msgsnd(msgid,&local_msg,(size_t)MSG_SIZE,0)) return -1; /*TODO handle failure*/
      break;
    }
      
    case MSG_TYPE_SWAP_GENERIC_POINTER:{
      void *temp_ptr;

      temp_ptr = *hot_swap_pointer_old;
      *hot_swap_pointer_old = *hot_swap_pointer_new;
      *hot_swap_pointer_new = temp_ptr;

      local_msg.msg_type = MSG_TYPE_FINISHED_GENERIC_POINTER;
      if (msgsnd(msgid,&local_msg,(size_t)MSG_SIZE,0)) return -1; /*TODO handle failure*/

      break;
    }

    case MSG_TYPE_STOP_ENGINE:
      return -3;
    
    }
  }

  /*route ALSA sequencer events*/
  while (1){

    int ret;

    ret = snd_seq_event_input(alsa_seq_client_handle,&curr_event);
    
    if (ret >= 0) {

      /*temporary hack to handle note-on, velocity 0 = note off events*/
      if ((curr_event->type == SND_SEQ_EVENT_NOTEON)&&(curr_event->data.note.velocity == 0))
	curr_event->type = SND_SEQ_EVENT_NOTEOFF;

      rt_event_map_route_event(curr_event);
      break;
    }

    if (ret <= 0) break;

  }

  /*iterate through rtobject instances in process list*/
  for (curr=0;curr<process_list_size;++curr){

      curr_instance = process_list[curr];

      /*switch (implementation_type)*/
      switch ( curr_instance->imp_type ){

      case RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT:{
	int i,j;
	signed short *ptr;
	sample_t* internal_buff;

	/*TODO: replace this with imp-type agnostic handling*/
	/*check that instance is active*/
	if (!curr_instance->control_list[0]) break;
 	
	/*this process is particular to the sample format used for ALSA*/
     
	internal_buff = curr_instance->data_port_list[0]->data;
	ptr = curr_instance->imp_data.extern_out_element.device_buffer;

	
	if (soundtank_engine->interleaved == SND_PCM_ACCESS_RW_INTERLEAVED){

	  j = 0;
	  /*only write if there is meaningfull data and output's not muted*/
	  if ((!curr_instance->data_port_list[0]->lazy_zero)&&\
	      (curr_instance->control_list[1] <= 0)){

	    for(i=0; i<numframes; ++i){
	      
	      /*scale by output volume*/
	      ptr[j] = (signed short) (curr_instance->control_list[2] *\
				       internal_buff[i]); 
	      j += soundtank_engine->num_channels;

	    }	    

	  }else{

	    for(i=0; i<numframes; ++i){
   
	      ptr[j] = 0;
	      j += soundtank_engine->num_channels;

	    }
      
	  }
	  
	}else{

	  /*non-interleaved case*/
	  if ((!curr_instance->data_port_list[0]->lazy_zero)&&\
	      (curr_instance->control_list[1] <= 0)){

	    for (i=0; i<numframes; ++i){
	      
	      /*scale by output volume*/
	      ptr[i] = (signed short) (curr_instance->control_list[2] *\
				       internal_buff[i]);
	  
	    }	  

	  }else{
	    
	    memset(ptr,0,numframes*sizeof(signed short));

	  }
	  
	}
	
	break;
	/*end of case RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT*/
      }

      case RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT:{
	jack_default_audio_sample_t *out = (jack_default_audio_sample_t*)\
	  jack_port_get_buffer(curr_instance->imp_data.jack_extern_element.jack_port, 0);
	
	/*push local input buffer to external output buffer*/
	if ((curr_instance->control_list[0] > 0)&&\
	    (curr_instance->control_list[1] <= 0)&&\
	    (!curr_instance->data_port_list[0]->lazy_zero)){
	  for (i=0; i<numframes; ++i)
	    out[i] = curr_instance->control_list[2] * curr_instance->data_port_list[0]->data[i];
	}else{
	  memset((void*)out, 0, sizeof(jack_default_audio_sample_t) * numframes);
	}
	
	break;
	/*end of case RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT */
      }

      case RTOBJECT_IMP_TYPE_JACK_EXTERN_IN:{
	jack_default_audio_sample_t *in = (jack_default_audio_sample_t*)\
	  jack_port_get_buffer(curr_instance->imp_data.jack_extern_element.jack_port, 0);
	
	/*push external input buffer to local output buffer*/
	if ((curr_instance->control_list[0] > 0)&&\
	    (curr_instance->control_list[1] <= 0)){
	  for (i=0; i<numframes; ++i)
	    curr_instance->data_port_list[0]->data[i] = curr_instance->control_list[2] * in[i];
	}
	curr_instance->data_port_list[0]->lazy_zero = 0;

	break;
	/*end of case RTOBJECT_IMP_TYPE_JACK_EXTERN_IN */
      }

      case RTOBJECT_IMP_TYPE_SIGNAL_PATH:{
	node_t* temp_node;

	/*signal path's must zero all their local buffers*/
	temp_node = curr_instance->imp_data.signal_path_element.local_buffer_list;
	
	for ( ;temp_node;temp_node=temp_node->next){
	  /*must manually zero the whole buffer in case it is attached
	    to a ladspa plugin's input port. They don't recognize our lazy zero-ing*/
	  memset((void*)((buffer_t*)temp_node->data)->data,\
		  0,\
		  ((buffer_t*)temp_node->data)->numsamples * sizeof(sample_t));
	  /* this is the lazy-zero method which doesn't work:
	     ((buffer_t*)temp_node->data)->lazy_zero = 0x1; 
	  */
	}

	break;
	/*end of case RTOBJECT_IMP_TYPE_SIGNAL_PATH */
      }

      case RTOBJECT_IMP_TYPE_INLINE:{


	/*TODO: replace this with imp-type agnostic handling*/
	/*check that instance is active*/
	if (!curr_instance->control_list[0]) break;

	switch ( curr_instance->major_type ){
	  
	case RTOBJECT_MAJOR_TYPE_LOCAL_OUT:{
 
	  /*push input buffer to output buffer*/
	  if (curr_instance->control_list[1] <= 0)
	    buffer_scale_push(curr_instance->data_port_list[0],\
			      curr_instance->data_port_list[1], \
			      curr_instance->control_list[2]);
	  break;
	  /*end of imp case RTOBJECT_MAJOR_TYPE_LOCAL_OUT*/
	}

	case RTOBJECT_MAJOR_TYPE_LOCAL_IN:{
 
	  /*push output buffer to input buffer*/
	  if (curr_instance->control_list[1] <= 0)
	    buffer_scale_push(curr_instance->data_port_list[0],\
			      curr_instance->data_port_list[1], \
			      curr_instance->control_list[2]);
	  break;
	  /*end of imp case RTOBJECT_MAJOR_TYPE_LOCAL_IN*/
	}
	  	  
	}
	  
	break;
	/*end of case RTOBJECT_IMP_TYPE_INLINE */
      }
      case RTOBJECT_IMP_TYPE_TEST_SOURCE:{
	sample_t* wr;
	int local_count = curr_instance->imp_data.test_source_element.curr_pos;

	/*TODO: replace this with imp-type agnostic handling*/
	/*check that instance is active*/
	if (!curr_instance->control_list[0]) break; 

	/*only run if unmuted (technically we should run anyway w/out volume)*/
	if (curr_instance->control_list[1] <= 0){

	  wr = (sample_t*)curr_instance->data_port_list[0]->data;
	  
	  for(i=0;i<numframes;++i){
	    if (++local_count > 100){
	      wr[i] = curr_instance->control_list[2];
	    }else{
	      wr[i] = -1 * curr_instance->control_list[2];
	    }
	    if (local_count>200) {
	      local_count = 0;
	    }
	  }

	  curr_instance->data_port_list[0]->lazy_zero = 0;
	  curr_instance->imp_data.test_source_element.curr_pos = local_count;

	}

	break;
	/*end of case RTOBJECT_IMP_TYPE_TEST_SOURCE */
      }

      case RTOBJECT_IMP_TYPE_LADSPA_PLUGIN:{

	/*activate the plugin if it's active and wasn't previously*/
	if ((curr_instance->control_list[0] > 0)&&(!curr_instance->active_before)){

	  /*LADSPA API CALL*/
	  /*call LADSPA activate function*/
	  if (curr_instance->imp_data.ladspa_plugin_element.descriptor->activate)
	    curr_instance->imp_data.ladspa_plugin_element.descriptor->\
	      activate(curr_instance->imp_data.ladspa_plugin_element.handle);
	  
	}

	/*run the plugin if it's active or was previously*/
	if ((curr_instance->control_list[0] > 0)||(curr_instance->active_before)){

	  /*LADSPA API CALL*/
	  /*run the LADSPA plugin to get all the data for this run*/
	  curr_instance->imp_data.ladspa_plugin_element.descriptor->\
	    run(curr_instance->imp_data.ladspa_plugin_element.handle, numframes);
	  
	}

	/*send data out as is if active & previously active & unmuted & previously unmuted*/
	if ((curr_instance->control_list[0] > 0)&&(curr_instance->active_before)&&\
	    (curr_instance->control_list[1] <= 0)&&(curr_instance->unmuted_before)){
	    
	  /*copy data from working buffers to output port buffers*/
	  for (temp_node=curr_instance->imp_data.ladspa_plugin_element.output_data_port_list;\
		 temp_node;\
		 temp_node=temp_node->next\
	       ){
	    
	    /*don't need to push data in overwrite mode*/
	    if (curr_instance->imp_data.ladspa_plugin_element.overwrite){
	      
	      ((data_port_t*)temp_node->data)->buffer_struct->lazy_zero = 0;
	      
	    }else{
	      
	      /*TODO: just set this once in get_working_buffer*/
	      ((data_port_t*)temp_node->data)->working_buffer->lazy_zero = 0;
	      
	      buffer_scale_push(((data_port_t*)temp_node->data)->working_buffer,\
				((data_port_t*)temp_node->data)->buffer_struct,\
				curr_instance->control_list[2]);
	    
	    }
	    
	  }
	  
	}else 
	  
	  /*three cases require ramping the data up to avoid clicking:
	    -1- active & previously active & unmuted & previously muted
	    -2- active & not previously active & unmuted & previously unmuted
	    -3- active & not previously active & unmuted & previously muted */
	if (((curr_instance->control_list[0] > 0)&&(curr_instance->active_before)&&\
	     (curr_instance->control_list[1] <= 0)&&(!curr_instance->unmuted_before))\
	    ||\
	    ((curr_instance->control_list[0] > 0)&&(!curr_instance->active_before)&&\
	     (curr_instance->control_list[1] <= 0)&&(curr_instance->unmuted_before))\
	    ||\
	    ((curr_instance->control_list[0] > 0)&&(!curr_instance->active_before)&&\
	     (curr_instance->control_list[1] <= 0)&&(!curr_instance->unmuted_before))){
	  
	  
	  /*copy data from working buffers to output port buffers, fade in to avoid clicks*/
	  for (temp_node=curr_instance->imp_data.ladspa_plugin_element.output_data_port_list;\
		 temp_node;\
		 temp_node=temp_node->next\
	       ){

	    /*don't need to push data in overwrite mode*/
	    if (curr_instance->imp_data.ladspa_plugin_element.overwrite){
	    
	      /*TODO: just set this once in get_working_buffer*/
	      ((data_port_t*)temp_node->data)->buffer_struct->lazy_zero = 0;
	      
	      buffer_fade_in(((data_port_t*)temp_node->data)->buffer_struct, 0, 100);

	    }else{

	      /*TODO: just set this once in get_working_buffer*/
	      ((data_port_t*)temp_node->data)->working_buffer->lazy_zero = 0;
	      
	      buffer_fade_in(((data_port_t*)temp_node->data)->working_buffer, 0, 100);
	      
	      buffer_scale_push(((data_port_t*)temp_node->data)->working_buffer,\
				((data_port_t*)temp_node->data)->buffer_struct,\
				curr_instance->control_list[2]);
	    }

	  }
	  
	}else

	  /*three cases require ramping the data down to avoid clicking:
	    -1- active & previously active & muted & previously unmuted
	    -2- inactive & previously active & unmuted & previously unmuted
	    -3- inactive & previously active & muted & previously unmuted */
	if (((curr_instance->control_list[0] > 0)&&(curr_instance->active_before)&&\
	     (curr_instance->control_list[1] > 0)&&(curr_instance->unmuted_before))\
	    ||\
	    ((curr_instance->control_list[0] <= 0)&&(curr_instance->active_before)&&\
	     (curr_instance->control_list[1] <= 0)&&(curr_instance->unmuted_before))\
	    ||\
	    ((curr_instance->control_list[0] <= 0)&&(curr_instance->active_before)&&\
	     (curr_instance->control_list[1] > 0)&&(curr_instance->unmuted_before))){
	  
	  
	  /*copy data from working buffers to output port buffers, fade out to avoid clicks*/
	  for (temp_node=curr_instance->imp_data.ladspa_plugin_element.output_data_port_list;\
		 temp_node;\
		 temp_node=temp_node->next\
	       ){
	    
	    /*don't need to push data in overwrite mode*/
	    if (curr_instance->imp_data.ladspa_plugin_element.overwrite){
	    
	      /*TODO: just set this once in get_working_buffer*/
	      ((data_port_t*)temp_node->data)->buffer_struct->lazy_zero = 0;
	      
	      buffer_fade_out(((data_port_t*)temp_node->data)->buffer_struct, 0, 100);

	    }else{

	      /*TODO: just set this once in get_working_buffer*/
	      ((data_port_t*)temp_node->data)->working_buffer->lazy_zero = 0;
	      
	      buffer_fade_out(((data_port_t*)temp_node->data)->working_buffer, 0, 100);
	      
	      buffer_scale_push(((data_port_t*)temp_node->data)->working_buffer,\
				((data_port_t*)temp_node->data)->buffer_struct,\
				curr_instance->control_list[2]);
	    }
	  
	  }

	}
	/*NOTE: no data is sent out from the plugin in the remaining 9 cases*/


	/*make previously-active flag align with current state*/
	if (curr_instance->control_list[0] > 0)
	  curr_instance->active_before = 0x1;
	else
	  curr_instance->active_before = 0x0;
	
	/*make previously-unmuted flag align with current state*/
	if (curr_instance->control_list[1] <= 0)
	  curr_instance->unmuted_before = 0x1;
	else
	  curr_instance->unmuted_before = 0x0;

	break;
	/*end of case RTOBJECT_IMP_TYPE_LADSPA_PLUGIN*/
      }

      case RTOBJECT_IMP_TYPE_CHANNEL_CP:{
 
	/*push output buffer to input buffer*/
	if (curr_instance->control_list[1] <= 0)
	  buffer_scale_push(curr_instance->data_port_list[0],\
			    curr_instance->data_port_list[1], \
			    curr_instance->control_list[2]);
	break;
	/*end of imp case RTOBJECT_IMP_TYPE_CHANNEL_CP*/
      }
      
      }

      /*debug*/
      if ((debug_process_function)&&(soundtank_engine->state != ENGINE_STATE_ACTIVE)){
	node_t* temp_node;

	printf("\nState of Soundtank's %d buffers after instance %d\n", \
	       ll_get_size(&buffer_list), curr);

	for (temp_node=buffer_list;temp_node;temp_node=temp_node->next){
	  buffer_debug_print(((buffer_t*)temp_node->data), debug_process_function);
	  printf("\n");
	}
	
	printf("\n");


      }
      

      
  }
      


  return 0;
}
