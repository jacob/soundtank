/*
 * Soundtank, a sound generation application
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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <ctype.h>

#define SOUNDTANK_STRUCTS_PRIMARY_REF yes
#include "include.h"
#include "soundtank_structs.h"



int main(int argc, const char** argv){

  /*init*/
  if ((soundtank_init(argc, argv)) < 0){
    printf("error: soundtank failed to initialize, aborting\n");
    return -1;
  }

  /*command line loop: read command, execute command*/
  while (1){
    soundtank_get_command();
    if ((soundtank_execute_command(dyn_command)) < 0) break;
  }

  /*cleanup*/ 
  /*TODO: deallocate _all_ allocated structs in cleanup function,
    otherwise, memory leak debugging is a pain*/
  soundtank_cleanup();

  /*done*/
  printf("\n  exiting Soundtank, goodbye\n\n");
  return 0;
}





rtobject_t* get_rtobject_from_address(int address){
  if ((address >= rtobject_address_list_size)||(address < 0)) return 0;
  return rtobject_address_list[address];
}

rtobject_t* get_rtobject_from_path(const char* name){
  rtobject_t* local_obj;
  signal_path_t* local_path;
  int local_indent;
  char* local_token;
  int ret;

  if (!(strlen(name))) return 0;
  
  /*if path begins with '&' then lookup as an address*/
  if (name[0] == '&'){
    int addr;
    addr = atoi(&name[1]);

    if ((!addr)&&(name[1] != '0')){
      printf("get robject from path error: '&' should precede an address ");
      printf("but you entered %s\n", &name[1]);
      return 0;
    }

    return get_rtobject_from_address(addr);
  }

  if ('/' == name[0]){
    /*absolute pathname*/
    local_indent = 1;
    local_obj = master_path_rtobject;
  }else{
    /*relative pathname*/
    local_indent = 0;
    local_obj = get_rtobject_from_address(signal_path_get_owner_rtobject_address(curr_path));
  }

  while ((ret = get_token_from_pathname(name,local_indent,&local_token))){

    if (ret < 0){
      printf("memory error\n");
      return 0;
    }

    /*advance to next token in pathname*/
    local_indent += ret;
    if ('/' == name[local_indent]) ++local_indent;
    
    /*find current sigpath from current obj*/
    if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(local_obj)){
      printf("lookup error, %s is not a signal path\n",rtobject_get_name(local_obj));
      return 0;
    }
    local_path = (signal_path_t*)local_obj->imp_struct;
    
    /*look up current token in current sigpath*/
    if ((strcmp(".",local_token))){
      
      if (!(strcmp("..",local_token))){
	local_path = rtobject_get_parent_path(local_obj);
	local_obj = get_rtobject_from_address(signal_path_get_owner_rtobject_address(local_path));
      }else{

	if (!(local_obj = signal_path_get_rtobject_from_name(local_path,local_token))){
	  free(local_token);
	  return 0;
	}

      }

    }

    /*get_token allocates a string*/
    free(local_token); 
  }

  return local_obj;
}

void release_address(int address){
  if ((address<0)||(address>=rtobject_address_list_size))
    printf("WARNING: attempt to released an invalid address, data corruption\n");
  else
    rtobject_address_list[address] = 0;
}

int get_free_address(){
  int new_address,i;

  new_address = -1;

  /*try to find a free address*/
  for(i=0;i<rtobject_address_list_size;++i){
    if (!rtobject_address_list[i]){
      new_address = i;
      break;
    }
  }

  /*increase address array if necessary*/
  if (new_address < 0){
    int addcount;
   
    addcount = 32;

    /*make new address array*/
    swap_rtobject_address_list = (rtobject_t**)malloc((rtobject_address_list_size+addcount)*sizeof(rtobject_t*));
    if (!swap_rtobject_address_list) return -1;
    swap_rtobject_address_list_size = rtobject_address_list_size+addcount;
    new_address = rtobject_address_list_size;
    
    /*fill new address array*/
    memcpy((void*)swap_rtobject_address_list,(void*)rtobject_address_list,(size_t)rtobject_address_list_size);
    for (i=rtobject_address_list_size;i<(rtobject_address_list_size+addcount);++i) swap_rtobject_address_list[i]=0;

    /*swap new address array with old one*/
    if (soundtank_engine->state == ENGINE_STATE_ACTIVE){
      /*must allow active engine to swap arrays like indiana jones in
	the temple of doom*/
      struct swap_message local_msg;
      local_msg.msg_type = MSG_TYPE_SWAP_ADDRESS_LIST;
      if (msgsnd(msgid,&local_msg,(size_t)MSG_SIZE,0)) return -1; 
      /*TODO handle removal of orphaned object*/
      /*wait until RT thread swaps arrays*/
      msgrcv(msgid,&local_msg,(size_t)MSG_SIZE,MSG_TYPE_FINISHED_ADDRESS_LIST,0);
    }else{
      int temp_size;
      rtobject_t** temp_swap;
      temp_swap = rtobject_address_list;
      temp_size = rtobject_address_list_size;
      rtobject_address_list = swap_rtobject_address_list;
      rtobject_address_list_size = swap_rtobject_address_list_size;
      swap_rtobject_address_list = temp_swap;
      swap_rtobject_address_list_size = temp_size;
    }
    
    /*deallocate old address array*/
    free(swap_rtobject_address_list);
    swap_rtobject_address_list_size=0; /*this is superficial, count on
					 swap array pointer itself*/
  }

  return new_address;
}

void release_buffer(int object_address, buffer_t* buff){
  buffer_remove_reference(buff, object_address);
}

buffer_t* get_free_buffer(){
  node_t* temp_node;
  node_t  node;
  static int buffer_address_count = 0;

  /*look for a pre-existing free buffer*/
  for (temp_node = buffer_list;temp_node;temp_node=temp_node->next){
    if (BUFF_FREE == buffer_get_flags((buffer_t*)temp_node->data))
      return (buffer_t*)temp_node->data;
  }
 
  temp_node = &node;

  /*no free buffer found so create one*/
  if (!(temp_node->data = (void*)buffer_alloca(buffer_address_count++, soundtank_engine->period_size)))
    return 0;
  if (!(temp_node =  ll_append(&buffer_list, temp_node->data))){
    buffer_dealloca((buffer_t*)temp_node->data);
    return 0;
  }

  if (buffer_address_count == 32000) buffer_address_count = 0; /*as bill the 
								 cat says: 
								 ack ack ack*/

  buffer_zero((buffer_t*)temp_node->data);

  return (buffer_t*)temp_node->data;
}


buffer_t* get_shared_buffer(int object_address){
  node_t* temp_node;
  buffer_t* new_buff;

  /*TODO: fix this up, nobody calls this anymore so it is irrelevant*/

  /*look for a pre-existing shared buffer*/
  for (temp_node = buffer_list;temp_node;temp_node=temp_node->next){
    if ((BUFF_SHARE == buffer_get_flags((buffer_t*)temp_node->data))&&\
	(!buffer_check_reference((buffer_t*)temp_node->data,object_address))){
      if ((buffer_add_reference((buffer_t*)temp_node->data,object_address,BUFF_SHARE))<0) return 0;
      return (buffer_t*)temp_node->data;
    }
  }
  
  /*couldn't find a shareable buffer so make one*/
  if (!(new_buff = get_free_buffer())) return 0;
  if ((buffer_add_reference(new_buff,object_address,BUFF_SHARE))<0) return 0;
  return new_buff;
}

buffer_t* get_working_buffer(int index){
  node_t* temp_node;
  buffer_t* new_buff;
  int i;

  /*
    Current imp: don't mark indexes of working buffer, just count on
    them being in order.  First working buffer in buffer_list is
    working buffer 0, next one is 1, etc.  Multiple working buffers
    are needed for rtobjects with multiple output ports.
  */

  /*look for a pre-existing working buffer with matching index*/
  i=0;
  for (temp_node = buffer_list;temp_node;temp_node=temp_node->next){
    if ((BUFF_WORKING == buffer_get_flags((buffer_t*)temp_node->data))&&(index == i++))
      return (buffer_t*)temp_node->data;
  }

  /*the requested working buffer doesn't exist yet, so make it*/
  if (!(new_buff = get_free_buffer())) return 0;
  if ((buffer_add_reference(new_buff,-1,BUFF_WORKING))<0) return 0;
  return new_buff;
}

int get_scale_list_size(){
  return ll_get_size(&scale_list);
}

scale_t* get_scale(int pos){
  node_t* temp_node;

  if (!(temp_node = ll_get_node(&scale_list, pos))){
    printf("get scale error: scale %d not found\n", pos);
    return 0;
  }

  return (scale_t*)temp_node->data;
}

scale_t* get_scale_by_name(const char *name){
  scale_t* curr_scale;
  node_t* temp_node;

  for (temp_node=scale_list;temp_node;temp_node=temp_node->next){
    curr_scale = (scale_t*)temp_node->data;

    if (!(strcmp(name, scale_get_name(curr_scale))))
      return curr_scale;

  }

  printf("get scale error: scale %s not found\n", name);
  return 0;
}

int add_scale(scale_t* new_scale){

  if (!(ll_append(&scale_list, (void*)new_scale))){
    printf("add scale error: memory error\n");
    return -1;
  }

  return 0;
}

int remove_scale(int pos){
  node_t* temp_node;

  if (!(temp_node = ll_get_node(&scale_list, pos))){
    printf("remove scale error: scale %d not found\n", pos);
    return 0;
  }

  ll_remove(temp_node, &scale_list);

  return 0;
}

int remove_scale_by_name(const char *name){
  scale_t* curr_scale;
  node_t* temp_node;

  for (temp_node=scale_list;temp_node;temp_node=temp_node->next){
    curr_scale = (scale_t*)temp_node->data;

    if (!(strcmp(name, scale_get_name(curr_scale))))
      break;
  }

  if (!temp_node){
    printf("get scale error: scale %s not found\n", name);
    return -1;
  }

  ll_remove(temp_node, &scale_list);

  return 0;
}

int remake_process_list(){
  int newsize;
  ll_head dirty_object_list = 0;

  /*find size for new process list*/
  newsize = signal_path_get_instance_count(master_path_rtobject);

  if (debug_readout) printf("remaking process list with size %d\n",newsize);

  /*allocate new process list*/
  if (newsize){ 
    
    if (!(swap_process_list = (rtobject_instance_t**)malloc(newsize * sizeof(rtobject_instance_t*)))){
      printf("error: could not allocate memory for new process list\n");
      return -1;
    }
    swap_process_list_size = newsize;

  }else{
    
    /*here is a hack to handle emptying the list*/
    swap_process_list = 0;
    swap_process_list_size = -1;

  }

  /*fill new process list*/
  /*NOTE: during this swap, all objects are checked for outdated info
    to be cleaned. See imp_objects/signal_path.c for an explanation*/
  if (swap_process_list)
    if  ((signal_path_fill_new_process_list(master_path_rtobject, swap_process_list, 0, &dirty_object_list)) < 0){
      printf("error: failed attempt to fill new process list\n");
      return -1;
    }

  /*swap old process list with new one*/
  if (soundtank_engine->state == ENGINE_STATE_ACTIVE){

    /*must have active engine swap arrays like indiana jones in the
      temple of doom*/
    struct swap_message local_msg;
    local_msg.msg_type = MSG_TYPE_SWAP_PROCESS_LISTS;
    if (msgsnd(msgid,&local_msg,(size_t)MSG_SIZE,0)) return -1;
    msgrcv(msgid,&local_msg,(size_t)MSG_SIZE,MSG_TYPE_FINISHED_PROCESS_LISTS,0);

  }else{

    rtobject_instance_t** temp_ptr;
    temp_ptr = process_list;
    process_list = swap_process_list;
    process_list_size = swap_process_list_size;
    /*hack to handle emptying the list*/
    if (process_list_size < 0) process_list_size = 0;
    swap_process_list = temp_ptr;
    
  }

  /*clean dirty objects*/
  while (dirty_object_list){
    if ((rtobject_cleanup((rtobject_t*)dirty_object_list->data)) < 0){
      printf("remake proc list error: failed attempt to clean object %d\n",\
	     rtobject_get_address((rtobject_t*)dirty_object_list->data));
    }
    ll_remove(dirty_object_list,&dirty_object_list);
  }

  /*free memory used by old process list*/
  free(swap_process_list);
  swap_process_list = 0;
  swap_process_list_size = 0;

  return process_list_size;
}


int pointer_hot_swap(void** old_ptr, void** new_ptr){
  /*this fxn synchronizes changes to live data structures with the
    realtime thread (engine)*/

  hot_swap_pointer_old = old_ptr;
  hot_swap_pointer_new = new_ptr;

  /*swap old pointer with new one*/
  if (soundtank_engine->state == ENGINE_STATE_ACTIVE){

    /*must have active engine swap pointers like indiana jones in the
      temple of doom*/
    struct swap_message local_msg;
    local_msg.msg_type = MSG_TYPE_SWAP_GENERIC_POINTER;
    if (msgsnd(msgid,&local_msg,(size_t)MSG_SIZE,0)) return -1;
    msgrcv(msgid,&local_msg,(size_t)MSG_SIZE,MSG_TYPE_FINISHED_GENERIC_POINTER,0);

  }else{
    
    void *temp_ptr;
    temp_ptr = *hot_swap_pointer_old;
    *hot_swap_pointer_old = *hot_swap_pointer_new;
    *hot_swap_pointer_new = temp_ptr;
    
  }

  return 0;
}
