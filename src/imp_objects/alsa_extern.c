/*
 * imp object ALSA extern in/out code
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


#include "../include.h"
#include "../soundtank_structs.h"


alsa_extern_t* imp_object_alsa_extern_alloca(){
  alsa_extern_t* newimp;
  if (!(newimp =  (alsa_extern_t*)malloc(sizeof(alsa_extern_t)))) return 0;
  newimp->owner_object_address=-1;
  newimp->device_channel=-1;
  newimp->device_buffer=0;
  return newimp;
}

void imp_object_alsa_extern_dealloca(alsa_extern_t* oldobj){

  engine_alsa_detach_from_device_output_channel(soundtank_engine, oldobj->device_channel, oldobj->owner_object_address);

  free(oldobj);
}


int create_imp_object_alsa_extern_out(rtobject_t* rtobj){
  data_port_t* curr_port;
  control_t* curr_control;
  alsa_extern_t* newimp;

  /*sanity check*/
  if (engine_get_method(soundtank_engine) != ENGINE_METHOD_ALSA){
    printf("alsa extern out error: Soundtank engine is not in ALSA mode\n");
    return -1;
  }

  /*check argc*/
  if (rtobj->imp_arg_list_size < 1){
    printf("alsa extern out imp object error: not enough imp arguments\n");
    return -1;
  }

  /*allocate imp object and attach it to rtobj*/
  if (!(newimp = imp_object_alsa_extern_alloca())){
    printf("alsa extern out imp object error: memory error\n");
    return -1;
  }
  newimp->owner_object_address = rtobj->address;
  rtobj->imp_struct = (void*)newimp;
  
  /*read device channel from implementation arguments*/
  newimp->device_channel = atoi(rtobj->imp_arg_list[0]);
  if (debug_readout) 
    printf("\nDEBUG: new extern has device channel %d\n", newimp->device_channel);

  /*attach to engine's device buffer to appropriate external channel*/
  if (!(newimp->device_buffer = \
	engine_alsa_attach_to_device_output_channel(soundtank_engine,\
						    newimp->device_channel,\
						    newimp->owner_object_address))){
    printf("alsa extern out imp object error: failed attempt to attach to engine channel");
    free(newimp);
    return -1;
  }


  /*make data ports: single input port*/
  if (!(curr_port = data_port_alloc(1))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }
  data_port_set_description_string(curr_port, "in");

  if (!(ll_append(&rtobj->data_port_list,(void*)curr_port))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }
  rtobj->data_port_list_size = ll_get_size(&rtobj->data_port_list);


  /*make 3 controls: active, mute and volume*/
 
  /*control 0: active*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_ACTIVE))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }
    
  /*control 1: mute*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_MUTE))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }
    
  /*control 2: volume*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_VOLUME))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }
    
  return 0;
}

int destroy_imp_object_alsa_extern_out(rtobject_t* rtobj){

  imp_object_alsa_extern_dealloca((alsa_extern_t*)rtobj->imp_struct);
  rtobj->imp_struct=0;

  return 0;
}

int create_imp_object_alsa_extern_in(rtobject_t* rtobj){

  return 0;
}

int destroy_imp_object_alsa_extern_in(rtobject_t* rtobj){

  rtobj->imp_struct=0;
  return 0;
}

int init_instance_alsa_extern_out(rtobject_t* rtobj, rtobject_instance_t* rtins){
  alsa_extern_t* alsa_ex;

  if (!(alsa_ex = (alsa_extern_t*)rtobj->imp_struct)) return -1;

  rtins->imp_data.extern_out_element.device_channel = alsa_ex->device_channel;
  rtins->imp_data.extern_out_element.device_buffer = alsa_ex->device_buffer;

  return 0;
}

int deinit_instance_alsa_extern_out(rtobject_t* rtobj, rtobject_instance_t* rtins){

  rtins->imp_data.extern_out_element.device_buffer = 0;

  return 0;
}


int init_instance_alsa_extern_in(rtobject_t* rtobj, rtobject_instance_t* rtins){

  return 0;
}

int deinit_instance_alsa_extern_in(rtobject_t* rtobj, rtobject_instance_t* rtins){


  return 0;
}

