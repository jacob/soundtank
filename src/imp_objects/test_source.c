/*
 * imp object test source code
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

#include "../include.h"


int create_imp_object_test_source(rtobject_t* rtobj){
  data_port_t* curr_port;
  control_t* curr_control;

  /*do not use implementation struct for anything*/
  rtobj->imp_struct = 0;


  /*make data ports: single output port*/
  if (!(curr_port = data_port_alloc(0))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }

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

int destroy_imp_object_test_source(rtobject_t* rtobj){

  rtobj->imp_struct=0;
  return 0;
}

int init_instance_test_source(rtobject_t* rtobj, rtobject_instance_t* rtins){

  rtins->imp_data.test_source_element.curr_pos = 0;

  return 0;
}

int deinit_instance_test_source(rtobject_t* rtobj, rtobject_instance_t* rtins){
  return 0;
}

int remove_ok_test_source(rtobject_t* rtobj){
  return 0;
}
