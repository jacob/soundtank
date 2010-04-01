/*
 * imp object JACK extern in/out code
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


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#include "../include.h"
#include "../soundtank_structs.h"




jack_extern_t* imp_object_jack_extern_alloca(){
  jack_extern_t* newimp;
  if (!(newimp =  (jack_extern_t*)malloc(sizeof(jack_extern_t)))) return 0;
  newimp->owner_object_address=-1;
  newimp->jack_port=0;
  return newimp;
}

void imp_object_jack_extern_dealloca(jack_extern_t* oldobj){
  free(oldobj);
}


int create_imp_object_jack_extern_out(rtobject_t* rtobj){
  data_port_t* curr_port;
  control_t* curr_control;
  jack_extern_t* newimp;
  jack_client_t* client;

  /*sanity check*/
  if (engine_get_method(soundtank_engine) != ENGINE_METHOD_JACK){
    printf("jack extern out error: Soundtank engine is not in JACK mode\n");
    return -1;
  }

  /*allocate imp object and attach it to rtobj*/
  if (!(newimp = imp_object_jack_extern_alloca())){
    printf("jack extern out imp object error: memory error\n");
    return -1;
  }
  newimp->owner_object_address = rtobj->address;
  rtobj->imp_struct = (void*)newimp;

  /*create a JACK output port*/
  client = engine_jack_get_client();
  if (!(newimp->jack_port = jack_port_register(client,\
					       rtobject_get_name(rtobj),\
					       JACK_DEFAULT_AUDIO_TYPE,\
					       JackPortIsOutput,\
					       0))){
    printf("jack extern out imp object error: failed to create jack port\n");
    return -1;
  }

  /*make data ports: single input port*/
  if (!(curr_port = data_port_alloc(1))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
  data_port_set_description_string(curr_port, "in");

  if (!(ll_append(&rtobj->data_port_list,(void*)curr_port))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
  rtobj->data_port_list_size = ll_get_size(&rtobj->data_port_list);


  /*make 3 controls: active, mute and volume*/
 
  /*control 0: active*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_ACTIVE))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
    
  /*control 1: mute*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_MUTE))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
    
  /*control 2: volume*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_VOLUME))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
    
  return 0;
}

int destroy_imp_object_jack_extern_out(rtobject_t* rtobj){
  jack_client_t* client;

  client = engine_jack_get_client();

  /*unregister JACK port*/
  jack_port_unregister(client, ((jack_extern_t*)rtobj->imp_struct)->jack_port);

  /*free up imp object*/
  imp_object_jack_extern_dealloca((jack_extern_t*)rtobj->imp_struct);
  rtobj->imp_struct=0;

  return 0;
}

int create_imp_object_jack_extern_in(rtobject_t* rtobj){
  data_port_t* curr_port;
  control_t* curr_control;
  jack_extern_t* newimp;
  jack_client_t* client;

  /*sanity check*/
  if (engine_get_method(soundtank_engine) != ENGINE_METHOD_JACK){
    printf("jack extern out error: Soundtank engine is not in JACK mode\n");
    return -1;
  }

  /*allocate imp object and attach it to rtobj*/
  if (!(newimp = imp_object_jack_extern_alloca())){
    printf("jack extern out imp object error: memory error\n");
    return -1;
  }
  newimp->owner_object_address = rtobj->address;
  rtobj->imp_struct = (void*)newimp;

  /*create a JACK output port*/
  client = engine_jack_get_client();
  if (!(newimp->jack_port = jack_port_register(client,\
					       rtobject_get_name(rtobj),\
					       JACK_DEFAULT_AUDIO_TYPE,\
					       JackPortIsInput,\
					       0))){
    printf("jack extern out imp object error: failed to create jack port\n");
    return -1;
  }

  /*make data ports: single output port*/
  if (!(curr_port = data_port_alloc(0))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
  data_port_set_description_string(curr_port, "out");

  if (!(ll_append(&rtobj->data_port_list,(void*)curr_port))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
  rtobj->data_port_list_size = ll_get_size(&rtobj->data_port_list);


  /*make 3 controls: active, mute and volume*/
 
  /*control 0: active*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_ACTIVE))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
    
  /*control 1: mute*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_MUTE))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
    
  /*control 2: volume*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_VOLUME))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object jack out error: memory error\n");
    return -1;
  }
    
  return 0;
}

int destroy_imp_object_jack_extern_in(rtobject_t* rtobj){
  jack_client_t* client;

  client = engine_jack_get_client();

  /*unregister JACK port*/
  jack_port_unregister(client, ((jack_extern_t*)rtobj->imp_struct)->jack_port);

  /*free up imp object*/
  imp_object_jack_extern_dealloca((jack_extern_t*)rtobj->imp_struct);
  rtobj->imp_struct=0;

  return 0;
}

int init_instance_jack_extern_out(rtobject_t* rtobj, rtobject_instance_t* rtins){
  jack_extern_t* jack_ex;

  if (!(jack_ex = (jack_extern_t*)rtobj->imp_struct)) return -1;

  rtins->imp_data.jack_extern_element.jack_port = jack_ex->jack_port;

  return 0;
}

int deinit_instance_jack_extern_out(rtobject_t* rtobj, rtobject_instance_t* rtins){

  rtins->imp_data.jack_extern_element.jack_port = 0;

  return 0;
}


int init_instance_jack_extern_in(rtobject_t* rtobj, rtobject_instance_t* rtins){
  jack_extern_t* jack_ex;

  if (!(jack_ex = (jack_extern_t*)rtobj->imp_struct)) return -1;

  rtins->imp_data.jack_extern_element.jack_port = jack_ex->jack_port;

  return 0;
}

int deinit_instance_jack_extern_in(rtobject_t* rtobj, rtobject_instance_t* rtins){

  rtins->imp_data.jack_extern_element.jack_port = 0;

  return 0;
}

