/*
 * rtobject code
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
#include <alsa/asoundlib.h>

#include "include.h"
#include "soundtank_structs.h"


/*used by namespace to get name of a control*/
static const char* control_name_lookup(node_t* lookup_node){
  return control_get_desc_string((control_t*)lookup_node->data);
}

/*used by namespace to get name of a control*/
static const char* data_port_name_lookup(node_t* lookup_node){
  return data_port_get_description_string((data_port_t*)lookup_node->data);
}



rtobject_t* rtobject_alloca(){
  rtobject_t* new_obj;
  new_obj = (rtobject_t*)malloc(sizeof(rtobject_t));
  if (!new_obj) return new_obj;
  new_obj->name=0;
  new_obj->description=0;
  new_obj->address=-1;
  new_obj->parent=0;
  new_obj->process_index=-1;
  new_obj->major_type=-1;
  new_obj->imp_type=-1;
  new_obj->imp_subtype=-1;
  new_obj->imp_struct=0;

  /*control list*/
  new_obj->control_list=0;
  if (!(new_obj->control_ns = create_namespace(&new_obj->control_list,
					       control_name_lookup))){
    printf("memory error\n");
    return 0;
  }

  /*data port list*/
  new_obj->data_port_list_size=0;
  new_obj->data_port_list=0;
  if (!(new_obj->data_port_ns = create_namespace(&new_obj->data_port_list,
						 data_port_name_lookup))){
    printf("memory error\n");
    return 0;
  }

  new_obj->instance_list_size=0;
  new_obj->instance_list=0;
  new_obj->outdated_instance_list=0;
  new_obj->event_map_list=0;
  return new_obj;
}

void rtobject_dealloca(rtobject_t* rtobj){
  int i;
  if (rtobj->name) free(rtobj->name);
  if (rtobj->description) free(rtobj->description);
  if (rtobj->imp_arg_list){
    for (i=0;i<rtobj->imp_arg_list_size;++i){
      if (rtobj->imp_arg_list[i]) free(rtobj->imp_arg_list[i]);
    }
    free(rtobj->imp_arg_list);
  }
  if (rtobj->imp_struct) free(rtobj->imp_struct);

  destroy_namespace(rtobj->control_ns);
  while (rtobj->control_list){
    control_dealloc((control_t*)rtobj->control_list->data);
    ll_remove(rtobj->control_list,&rtobj->control_list);
  }

  destroy_namespace(rtobj->data_port_ns);
  while (rtobj->data_port_list){
    data_port_dealloc((data_port_t*)rtobj->data_port_list->data);
    ll_remove(rtobj->data_port_list,&rtobj->data_port_list);
  }

  if (rtobj->instance_list) free(rtobj->instance_list);

  rtobject_flush_map_list(rtobj);
  if (rtobj->event_map_list)
    generic_array_destroy(rtobj->event_map_list);

  free(rtobj);
}



int create_rtobject(int major_type, int imp_type, \
		    int imp_argc, const char** imp_argv, \
		    char* name, char* description, \
		    signal_path_t* target_path){
  rtobject_t* new_obj;
  node_t* new_obj_node;
  int i;

  /*error check types*/
  if (validate_major_type(major_type,imp_type)){
    if (debug_readout) printf("create rtobject error: failed major type validation\n");
    return -1;
  }

  /*allocate structure*/
  if (!(new_obj = rtobject_alloca())) return -1;

  /*fill core data*/
  new_obj->process_index = -1;
  new_obj->major_type = major_type;
  new_obj->imp_type = imp_type;
  new_obj->imp_arg_list_size = imp_argc;
  if (!(new_obj->name = strdup(name))){
    rtobject_dealloca(new_obj);
    return -1;
  }
  if (!(new_obj->description = strdup(description))){
    rtobject_dealloca(new_obj);
    return -1;
  }

  /*duplicate imp args*/
  if (!imp_argc){
    new_obj->imp_arg_list = 0;
  }else{

    if (!(new_obj->imp_arg_list = (char**)malloc(new_obj->imp_arg_list_size * sizeof(char*)))){
      rtobject_dealloca(new_obj);
      return -1;
    }

    for (i=0;i<new_obj->imp_arg_list_size;++i){
      if (!(new_obj->imp_arg_list[i] = strdup(imp_argv[i]))){
	rtobject_dealloca(new_obj);
	return -1;
      }
    }

  }

  /*create empty event map list*/
  if (!(new_obj->event_map_list = generic_array_create(sizeof(void*)))){
    rtobject_dealloca(new_obj);
    return -1;
  }

  /*assign address*/
  if ((new_obj->address = get_free_address())<0) {
    rtobject_dealloca(new_obj);
    return -1;
  }

  /*attach new rtobject to its address*/
  rtobject_address_list[new_obj->address] = new_obj;

  if (debug_readout) 
    printf("PUT IN OBJECT ADDRESSING ARRAY AT POSITION %d\n",new_obj->address);

 
  /*create imp_object -> controls, data ports */
  if ((create_rtobject_imp_object(new_obj))<0) {
    if (debug_readout) printf("create rtobject error: failed to create imp object\n");
    release_address(new_obj->address);
    rtobject_dealloca(new_obj);
    return -1;
  }

  /*attach data ports to null buffers*/
  for (i=0;i<rtobject_get_data_port_list_size(new_obj);++i){
    if ((rtobject_detach_port(new_obj,i)) < 0){
      printf("create rtobject error: could not attach port %d to null channel\n",i);
      release_address(new_obj->address);
      rtobject_dealloca(new_obj);
      return -1;
    }
  }
    
  if (debug_readout) printf("TRYING TO CREATE AN INSTANCE\n");
  
  /*create one instance*/
  if ((create_rtobject_instance(new_obj))<0) {
    release_address(new_obj->address);
    rtobject_dealloca(new_obj);
    return -1;
  }

  /*little bit of a hack until i figure out how to handle rtobject-instance*/
  if ((rtobject_update_instance_situation(new_obj)) < 0){
    release_address(new_obj->address);
    rtobject_dealloca(new_obj);
    return -1;
  }


  if (debug_readout) printf("CREATED AN INSTANCE\n");


  /*put rtobject into current path*/ /* * * * * **/

  if (!(strcmp("main",rtobject_get_name(new_obj)))){ 

    /*can't put master path into itself*/
    /*but master path is its own parent (try _that_ at home)*/
    new_obj->parent = (signal_path_t*)new_obj->imp_struct;

  }else{

    if (!(new_obj_node = (node_t*)malloc(sizeof(node_t)))){
      release_address(new_obj->address);
      rtobject_dealloca(new_obj);
      return -1;
    }
    new_obj_node->previous = new_obj_node->next = 0;
    new_obj_node->data = (void*)new_obj;

    if ((signal_path_insert(target_path, new_obj_node, -1)) < 0){
      release_address(new_obj->address);
      rtobject_dealloca(new_obj);
      return -1;
    }
  }

  /*TODO: REMOVE THIS, handled by inserting in a path*/
  /*create an ALSA sequencer port for rtobject*/
  /*the port index must be the same as the address array index*/
  /*TODO: fix to handle more than 256 rtobjects by using multiple alsaseq clients*/
  if ((rtobject_create_alsa_seq_port(new_obj)) < 0){
      release_address(new_obj->address);
      rtobject_dealloca(new_obj);
      return -1;
  }

  /*done*/
  return new_obj->address;
}


int create_rtobject_dup(rtobject_t* old_obj, char* name, signal_path_t* target_path){
  rtobject_t* new_obj;
  node_t* new_obj_node;
  node_t* temp_node;
  int i;

  /*error check: name can not be same as original object's*/
  if (!(strcmp(name,rtobject_get_name(old_obj)))){
    printf("duplicate rtobject error: new object must have different name\n");
    return -1;
  }

  /*allocate structure*/
  if (!(new_obj = rtobject_alloca())) return -1;


  /*fill core data*/
  new_obj->process_index = -1;
  new_obj->major_type = rtobject_get_major_type(old_obj);
  new_obj->imp_type = rtobject_get_implementation_type(old_obj);
  new_obj->imp_arg_list_size = rtobject_get_implementation_arg_list_size(old_obj);
  if (!(new_obj->name = strdup(name))){
    rtobject_dealloca(new_obj);
    return -1;
  }
  if (!(new_obj->description = strdup(rtobject_get_description(old_obj)))){
    rtobject_dealloca(new_obj);
    return -1;
  }

  /*duplicate imp args*/
  if (!new_obj->imp_arg_list_size){
    new_obj->imp_arg_list = 0;
  }else{

    if (!(new_obj->imp_arg_list = (char**)malloc(new_obj->imp_arg_list_size * sizeof(char*)))){
      rtobject_dealloca(new_obj);
      return -1;
    }

    for (i=0;i<new_obj->imp_arg_list_size;++i){
      if (!(new_obj->imp_arg_list[i] = strdup(old_obj->imp_arg_list[i]))){
	rtobject_dealloca(new_obj);
	return -1;
      }
    }

  }

  /*create empty event map list*/
  if (!(new_obj->event_map_list = generic_array_create(sizeof(void*)))){
    rtobject_dealloca(new_obj);
    return -1;
  }

  /*assign address*/
  if ((new_obj->address = get_free_address())<0) {
    rtobject_dealloca(new_obj);
    return -1;
  }

  /*attach new rtobject to its address*/
  rtobject_address_list[new_obj->address] = new_obj;

  if (debug_readout) printf("PUT IN OBJECT ADDRESSING ARRAY AT POSITION %d\n",new_obj->address);

 
  /*create imp_object -> controls, data ports */
  if ((create_rtobject_imp_object(new_obj))<0) {
    if (debug_readout) printf("create rtobject error: failed attempt to create imp object\n");
    release_address(new_obj->address);
    rtobject_dealloca(new_obj);
    return -1;
  }

  /*attach data ports to null buffers*/
  for (i=0;i<rtobject_get_data_port_list_size(new_obj);++i){
    if ((rtobject_detach_port(new_obj,i)) < 0){
      printf("create rtobject error: could not attach port %d to null channel\n",i);
      release_address(new_obj->address);
      rtobject_dealloca(new_obj);
      return -1;
    }
  }
    
  if (debug_readout) printf("TRYING TO CREATE AN INSTANCE\n");

  /*duplicate instances*/
  for (temp_node=old_obj->instance_list;temp_node;temp_node=temp_node->next){

    if ((create_rtobject_instance_copy(new_obj, (rtobject_instance_t*)temp_node->data))<0) {
      release_address(new_obj->address);
      rtobject_dealloca(new_obj);
      return -1;
    }

  }

  /*little bit of a hack until i figure out how to handle rtobject-instance*/
  /*i think we can get rid of this because putting it in sigpath handles it
  if ((rtobject_update_instance_situation(new_obj)) < 0){
    release_address(new_obj->address);
    rtobject_dealloca(new_obj);
    return -1;
  }
  */

  if (debug_readout) printf("CREATED AN INSTANCE\n");


  /*put rtobject into specified path*/

  if (!(strcmp("main",rtobject_get_name(new_obj)))){ 

    /*this is totally illegal for a variety of obvious reasons*/ 
    printf("WARNING: Illegal Call, Can NOT Duplicate Master Path!!!!!!\n");
    release_address(new_obj->address);
    rtobject_dealloca(new_obj);
    return -1;
   
  }else{

    /*TODO: this is awkward, should be a cleaner method*/
    if (!(new_obj_node = (node_t*)malloc(sizeof(node_t)))){
      release_address(new_obj->address);
      rtobject_dealloca(new_obj);
      return -1;
    }
    new_obj_node->previous = new_obj_node->next = 0;
    new_obj_node->data = (void*)new_obj;

    if ((signal_path_insert(target_path, new_obj_node, -1)) < 0){
      release_address(new_obj->address);
      rtobject_dealloca(new_obj);
      return -1;
    }
  }

  /*TODO: REMOVE THIS, handled by inserting in a path*/
  /*create an ALSA sequencer port for rtobject*/
  /*the port index must be the same as the address array index*/
  /*TODO: fix to handle more than 256 rtobjects by using multiple alsaseq clients*/
  if ((rtobject_create_alsa_seq_port(new_obj)) < 0){
      release_address(new_obj->address);
      rtobject_dealloca(new_obj);
      return -1;
  }

  /*done*/
  return new_obj->address;
}


void destroy_rtobject(rtobject_t* rtobj){
   node_t* temp_node;
   node_t* ret_node;

   /*sanity check: make sure we're not removing signal path unwisely*/
   if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(rtobj)){

     if ((((signal_path_t*)rtobj->imp_struct)->object_list)){
       printf("error: can not destroy signal path with members\n");
       return;
     }

     if (curr_path == (signal_path_t*)rtobj->imp_struct){
       printf("error: can not destroy signal path while inside of it\n");
       return;
     }

   }

   /*remove object from parent path*/
   if (!rtobj->parent){
    printf("destroy rtobject error: rtobject is not in a signal path: data is already corrupted\n");
    return;
  }

  if (!(temp_node = signal_path_get_node(rtobj->parent, rtobj))){
    printf("destroy rtobject error: failed to get object from it's parent, data is already corrupted\n");
    return;
  }
    
  if ((signal_path_remove(rtobj->parent, temp_node, 1, &ret_node)) < 0){
    printf("destroy rtobj err: failed attempt to remove object from parent path: Data Corruption\n");
    return;
  }
  
  free(ret_node);

  /*destroy alsa sequencer port*/
  {
    int port_id;
    port_id = rtobject_get_address(rtobj);

    /*alsa only allows ports from 0 to 256, this is silly but we can only work around*/
    if ((port_id >= 0)&&(port_id < 256)){

      snd_seq_delete_port(alsa_seq_client_handle, port_id);

    }
  }
 
  /*destroy all instances*/
  while (rtobj->instance_list){
    if ((destroy_rtobject_instance(rtobj)) < 0){
      printf("WARNING: failed to free all instances for rtobject being freed, memory leaked\n");
      break;
    }
  }

  /*deinitialize implementation object*/
  if ((destroy_rtobject_imp_object(rtobj)) < 0)
    printf("WARNING: failed to free implementation object for rtobject being freed, memory leaked\n");
  
  /*remove object from addressing system*/
  release_address(rtobject_get_address(rtobj));

  /*deallocate object*/
  rtobject_dealloca(rtobj);
}

const char* rtobject_get_name(rtobject_t* rtobj){return rtobj->name;}

int rtobject_set_name(rtobject_t* rtobj, const char* new_name){
  int first_time;

  if (rtobj->name){
    free(rtobj->name);
    first_time = 0;
  }else{
    first_time = 1;
  }

  if (!(rtobj->name = (char*)malloc((1 + strlen(new_name)) * sizeof(char)))){
    return -1;
  }

  strcpy(rtobj->name, new_name);

  return 0;
}

const char* rtobject_get_description(rtobject_t* rtobj){
  return rtobj->description;
}

int rtobject_set_description(rtobject_t* rtobj, const char* new_description){
  if (rtobj->description) free(rtobj->description);
  rtobj->description = \
    (char*)malloc((1 + strlen(new_description)) * sizeof(char));
  if (!rtobj->description) return -1;
  strcpy(rtobj->description, new_description);
  return 0;
}

int rtobject_get_address(const rtobject_t* rtobj){
  return rtobj->address;
}

int rtobject_get_process_index(const rtobject_t* rtobj){
  /*return process index of first instance*/
  rtobject_instance_t* ins;

  if (!(ins = rtobject_get_instance(rtobj, 0))) return -1;

  return ins->process_index;
}

rtobject_t* rtobject_get_parent(const rtobject_t* rtobj){
  return get_rtobject_from_address(signal_path_get_owner_rtobject_address(rtobj->parent));
}

signal_path_t* rtobject_get_parent_path(const rtobject_t* rtobj){
  return rtobj->parent;
}

int rtobject_get_major_type(const rtobject_t* rtobj){
  return rtobj->major_type;
}

int rtobject_get_implementation_type(const rtobject_t* rtobj){
  return rtobj->imp_type;
}

int rtobject_get_implementation_arg_list_size(const rtobject_t* rtobj){
  return rtobj->imp_arg_list_size;
}

const char* rtobject_get_implementation_arg(const rtobject_t* rtobj,\
					    int whicharg){
  if ((whicharg<0)||(whicharg>=rtobj->imp_arg_list_size)) return 0;
  return rtobj->imp_arg_list[whicharg];
}

int rtobject_get_control_list_size(const rtobject_t* rtobj){
  return ll_get_size(&rtobj->control_list);
}

const control_t* rtobject_get_control(const rtobject_t* rtobj, int pos){
  node_t* temp_node;
  if (!(temp_node = ll_get_node(&rtobj->control_list, pos)))
    return 0;
  return (const control_t*)temp_node->data;
}

int rtobject_get_control_index(const rtobject_t* rtobj, const char* name){
  return ns_search_pos(rtobj->control_ns, name, 0, SEARCH_EXACT);
}

int rtobject_search_controls(const rtobject_t* rtobj,\
			     const char *search_pattern,\
			     int search_type, ll_head* results){
  int ret, state;
  int *curr_int;

  /*sanity check*/
  if (*results){
    printf("search controls error: result list not intially empty\n");
    return -1;
  }

  /*check if search pattern is just a control index*/
  if (string_is_number(search_pattern) != 0){
    
    /*validate control index*/
    if (!(rtobject_get_control(rtobj, atoi(search_pattern)))){
      return -1;
    }

    /*allocate an integer*/
    if (!(curr_int = (int*)malloc(sizeof(int)))){
      return -1;
    }

    *curr_int = atoi(search_pattern);

    /*append integer to result list*/
    if (!(ll_append(results, (void*)curr_int))){
      free(curr_int);
      return -1;
    }
     
    return 1;
  }

  /*search control names, appending matching indexes to result list*/
  state = 0;
  while ((ret = ns_search_pos(rtobj->control_ns, search_pattern,\
			      state, search_type)) >= 0){

    /*allocate an integer*/
    if (!(curr_int = (int*)malloc(sizeof(int)))){
      ll_free_all(results);
      return -1;
    }

    *curr_int = ret;

    /*append integer to result list*/
    if (!(ll_append(results, (void*)curr_int))){
      ll_free_all(results);
      free(curr_int);
      return -1;
    }
    
    ++state;
  }

  return ll_get_size(results);
}

void rtobject_free_search_results(ll_head* results){
  while (*results){
    free((int*)(*results)->data);
    ll_remove(*results,results);
  }
}

int rtobject_get_data_port_list_size(const rtobject_t* rtobj){
  if ((RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(rtobj)))
    return rtobj->data_port_list_size;
  
  return signal_path_get_exposed_port_count((signal_path_t*)rtobj->imp_struct);
}

data_port_t* rtobject_get_data_port(const rtobject_t* rtobj, int port_index){
  node_t* temp_node;
  int i;

  if ((RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(rtobj))){
    if ((port_index < 0)||(port_index >= rtobj->data_port_list_size)) return 0;
    temp_node = rtobj->data_port_list;
    for (i=0;i<port_index;++i) temp_node=temp_node->next;
    if (!temp_node) return 0;
    return (data_port_t*)temp_node->data;
  }

  return signal_path_get_exposed_port((signal_path_t*)rtobj->imp_struct,port_index);
}

rtobject_t* rtobject_get_data_port_owner_rtobject(rtobject_t* rtobj, int port_index){
  /*you only need to use this fxn if rtobj is a signal path*/
  if ((RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(rtobj)))
    return rtobj;

  return signal_path_get_exposed_port_owner_rtobject((signal_path_t*)rtobj->imp_struct,port_index);
}

int rtobject_get_data_port_index(const rtobject_t* rtobj, const char* name){
  return ns_search_pos(rtobj->data_port_ns, name, 0, SEARCH_EXACT);
}

int rtobject_search_data_ports(const rtobject_t* rtobj,\
			     const char *search_pattern,\
			     int search_type, ll_head* results){
  int ret, state;
  int *curr_int;

  /*sanity check*/
  if (*results){
    printf("search data_ports error: result list not intially empty\n");
    return -1;
  }

  /*check if search pattern is just a data_port index*/
  if (string_is_number(search_pattern) != 0){
    
    /*validate data_port index*/
    if (!(rtobject_get_data_port(rtobj, atoi(search_pattern)))){
      return -1;
    }

    /*allocate an integer*/
    if (!(curr_int = (int*)malloc(sizeof(int)))){
      return -1;
    }

    *curr_int = atoi(search_pattern);

    /*append integer to result list*/
    if (!(ll_append(results, (void*)curr_int))){
      free(curr_int);
      return -1;
    }
     
    return 1;
  }

  /*search data_port names, appending matching indexes to result list*/
  state = 0;
  while ((ret = ns_search_pos(rtobj->data_port_ns, search_pattern,\
			      state, search_type)) >= 0){

    /*allocate an integer*/
    if (!(curr_int = (int*)malloc(sizeof(int)))){
      ll_free_all(results);
      return -1;
    }

    *curr_int = ret;

    /*append integer to result list*/
    if (!(ll_append(results, (void*)curr_int))){
      ll_free_all(results);
      free(curr_int);
      return -1;
    }
    
    ++state;
  }

  return ll_get_size(results);
}

int rtobject_attach_port_to_channel(rtobject_t* rtobj,\
				    int port_index, \
				    channel_t* chan){
  data_port_t* dport;
  rtobject_t* dport_owner_rtobj;

  if (!(dport = rtobject_get_data_port(rtobj, port_index))){
    printf("attach port error: couldn't find port #%d\n",port_index);
    return -1;
  }

  if (!(dport_owner_rtobj = rtobject_get_data_port_owner_rtobject(rtobj, port_index))){
    printf("attach port error: couldn't find port owner rtobject\n");
    return -1;
  }

  /*attach data port to channel*/
  if ((data_port_set_channel(dport,\
			     chan,\
			     rtobject_get_address(dport_owner_rtobj))) < 0){
    printf("rtobject attach port error: ");
    printf("failed attempt to set port's channel attachment\n");
    return -1;
  }

 /*update object's instance situation*/
  rtobject_update_instance_situation(dport_owner_rtobj);

  return 0;
}

int rtobject_detach_port(rtobject_t* rtobj, int port_index){
  data_port_t* dport;

  if (!(dport = rtobject_get_data_port(rtobj, port_index))){
    printf("detach port error: couldn't find port #%d\n",port_index);
    return -1;
  }

  if ((data_port_get_input(dport)))
    return rtobject_attach_port_to_channel(rtobj,port_index,null_read);
  else
    return rtobject_attach_port_to_channel(rtobj,port_index,null_write);

  data_port_set_target(dport,"",-1);
}

int rtobject_get_instance_list_size(const rtobject_t* rtobj){
  return rtobj->instance_list_size;
}

rtobject_instance_t* rtobject_get_instance(const rtobject_t* rtobj, int pos){
  node_t* temp_node;

  if (!(temp_node = ll_get_node(&rtobj->instance_list, pos))){
    return 0;
  }

  return (rtobject_instance_t*)temp_node->data;
}

int rtobject_update_instance_situation(rtobject_t* rtobj){
  int j;
  rtobject_instance_t* new_ins;
  node_t* temp_node;

  if (debug_readout) printf("updating instance situation for rtobject %d\n",\
			    rtobject_get_address(rtobj));

  /*
    If nothing is in the object's outdated instance list, we move all
    the current instances there, then duplicate all the current
    instances to form a new list of instances and modify the copies.
    Otherwise, this has been done already, but changes haven't been
    enacted and can be overwritten.  In either case we take the "new"
    instances and update their data, while leaving the "old" instances
    to be deallocated after the proclist is swapped out.
  */

  if (!rtobj->outdated_instance_list){

    /*swap instance list with outdated instance list*/
    rtobj->outdated_instance_list = rtobj->instance_list;
    rtobj->instance_list = 0;

    /*duplicate all current instances*/
    for (temp_node=rtobj->outdated_instance_list;\
	   temp_node;\
	   temp_node=temp_node->next){

      /*use copy allocator to duplicate each instance*/
      if (!(new_ins = rtobject_instance_alloca_copy((rtobject_instance_t*)temp_node->data))){
	printf("memory allocation error\n");
	return -1;
      }
          
      /*add duplicated instance to instance list*/
      if (!(ll_append(&rtobj->instance_list, (void*)new_ins))){
	printf("memory allocation error\n");
	return -1;
      }

    }

  }

  /*update new instances' data port lists*/
  for (temp_node=rtobj->instance_list;temp_node;temp_node=temp_node->next){
    new_ins = (rtobject_instance_t*)temp_node->data;
    
    for (j=0;j<rtobj->data_port_list_size;++j)
      new_ins->data_port_list[j] = data_port_get_buffer_struct(rtobject_get_data_port(rtobj,j));
    
  }

  /*handle implementation type specific instance updating*/
  /*TODO: make this done via uniformly named xxx_instance_attach fxn*/

  if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(rtobj)){

    for (temp_node=rtobj->instance_list;temp_node;temp_node=temp_node->next){
      new_ins = (rtobject_instance_t*)temp_node->data;
      
      /*erase old changes, will recreate*/
      while (new_ins->imp_data.signal_path_element.local_buffer_list){
	ll_remove(new_ins->imp_data.signal_path_element.local_buffer_list,\
		  &new_ins->imp_data.signal_path_element.local_buffer_list);
      }
     
      /*now we have a path instance w/ a blank context buffer list to
	be filled, fill it*/
      if ((init_instance_signal_path(rtobj,new_ins)) < 0){
	printf("failed attempt to initialize signal path instance\n");
	return -1;
      }

    }

  }

  if (RTOBJECT_IMP_TYPE_LADSPA_PLUGIN == rtobject_get_implementation_type(rtobj)){

    for (temp_node=rtobj->instance_list;temp_node;temp_node=temp_node->next){
      new_ins = (rtobject_instance_t*)temp_node->data;

      /*must call ladspa's connect port function to update plugin instance*/
      if ((attach_instance_ladspa_plugin(rtobj, new_ins)) < 0){
	printf("failed attempt to initialize LADSPA plugin instance\n");
	return -1;
      }

    }

  }

  return 0;
}

int rtobject_cleanup(rtobject_t* rtobj){
  node_t* temp_node;
  rtobject_instance_t* curr_ins;

  if (rtobject_get_major_type(rtobj) == RTOBJECT_MAJOR_TYPE_SIGNAL_PATH){

    /*free local buffer references from outdated instances*/
    for (temp_node=rtobj->outdated_instance_list;\
	   temp_node;\
	   temp_node=temp_node->next){

      curr_ins = (rtobject_instance_t*)temp_node->data;
      
      while (curr_ins->imp_data.signal_path_element.local_buffer_list){
	ll_remove(curr_ins->imp_data.signal_path_element.local_buffer_list,\
		  &curr_ins->imp_data.signal_path_element.local_buffer_list);
      }
      
    }

    /*call signal path's cleanup function to handle unused channels*/
    if ((signal_path_cleanup((signal_path_t*)rtobj->imp_struct)) <0) return -1;

  }
   
  /*free outdated instances*/
  while (rtobj->outdated_instance_list){
    free((rtobject_instance_t*)rtobj->outdated_instance_list->data);
    ll_remove(rtobj->outdated_instance_list, &rtobj->outdated_instance_list);
  }


  return 0;
}

char* rtobject_get_absolute_pathname(rtobject_t* rtobj){
  /*bloatware fxn that allocates a new string containing absolute
    pathname of rtobject*/
  rtobject_t* local_rtobj;
  ll_head pwd_list;
  node_t* temp_node;
  int path_len,i;
  char *path;

  pwd_list = 0;

  /*first find rtobject's path rtobject*/
  local_rtobj = rtobject_get_parent(rtobj);
   
  /*work up tree towards root, adding each name to a linked list*/
  while (!(signal_path_is_master_path((signal_path_t*)local_rtobj->imp_struct))){

    ll_append(&pwd_list,(void*)local_rtobj);
    local_rtobj = rtobject_get_parent(local_rtobj);

  }
   
  /*walk list to find length of pathname, remember pathname includes name*/
  /*precede each object name with '/' */
  path_len = 1 + strlen(rtobject_get_name(rtobj));
  for (temp_node=pwd_list;temp_node;temp_node=temp_node->next){
    path_len += (1 + strlen(rtobject_get_name((rtobject_t*)temp_node->data)));
  }

  /*remember ending null*/
  path_len++;

  /*allocate string to hold pathname*/
  if (!(path = (char*)malloc(path_len * sizeof(char)))){
    printf("error rtobject get absolute pathname memory error\n");
    return 0;
  }
    
  /*walk list backwards to fill path*/
  i=0;
  if (pwd_list){
    temp_node = pwd_list;
    while(temp_node->next) temp_node=temp_node->next;
    for (;temp_node;temp_node=temp_node->previous){
      path[i++] = '/';
      strncpy(&path[i],\
	      rtobject_get_name((rtobject_t*)temp_node->data),\
	      strlen(rtobject_get_name((rtobject_t*)temp_node->data)));
      i += strlen(rtobject_get_name((rtobject_t*)temp_node->data));
    }
  }

  /*rtobject name*/
  path[i++] = '/';
  strncpy(&path[i], rtobject_get_name(rtobj), strlen(rtobject_get_name(rtobj)));
  path[path_len - 1] = '\0'; /*strncpy seems to be f-ing this up*/

  /*free the list*/
  while (pwd_list) ll_remove(pwd_list,&pwd_list);

  return path;
}


int rtobject_create_alsa_seq_port(rtobject_t* rtobj){
  snd_seq_port_info_t* port_info_ptr;
  int port_id;
  char *ret;

  port_id = rtobject_get_address(rtobj);

  if ((RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(rtobj))&&\
      (!(strcmp("main", rtobject_get_name(rtobj))))){

    ret = strdup("main");

  }else{

    ret = rtobject_get_absolute_pathname(rtobj);

  }

  /*alsa only allows ports from 0 to 256, this is silly but we can only work around*/
  if ((port_id < 0)||(port_id > 256)) return -1;

  /*create an empty port descriptor structure*/
  if ((snd_seq_port_info_malloc(&port_info_ptr)) < 0)
    return -1;

  /*set port id*/
  snd_seq_port_info_set_port(port_info_ptr, port_id);

  /*set port name*/
  snd_seq_port_info_set_name(port_info_ptr, (const char*)ret);

  /*set capabilities*/
  snd_seq_port_info_set_capability(port_info_ptr,\
				     SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE);

  /*set type*/
  snd_seq_port_info_set_type(port_info_ptr,SND_SEQ_PORT_TYPE_MIDI_GENERIC);

  /*create the port*/
  if ((snd_seq_create_port(alsa_seq_client_handle, port_info_ptr)) < 0) 
    return -1;

  /*free port descriptor structure*/
  snd_seq_port_info_free(port_info_ptr);

  free(ret);

  return 0;
}


int rtobject_update_alsa_seq_port_name(rtobject_t* rtobj){
  snd_seq_port_info_t* port_info_ptr;
  int port_id;
  char *ret;
  
  port_id = rtobject_get_address(rtobj);
  if (port_id < 0){
    return -1;
  }
  if (!port_id)
    printf("modifying master path ALSA seq port !\n");
  
  ret = rtobject_get_absolute_pathname(rtobj);
    
  /*create an empty port descriptor structure*/
  if ((snd_seq_port_info_malloc(&port_info_ptr)) < 0)
    return -1;
  
  /*set port id*/
  snd_seq_port_info_set_port(port_info_ptr, port_id);
  
  /*set port name*/
  snd_seq_port_info_set_name(port_info_ptr, ret);
  
  /*set capabilities*/
  snd_seq_port_info_set_capability(port_info_ptr,\
				   SND_SEQ_PORT_CAP_WRITE|\
				   SND_SEQ_PORT_CAP_SUBS_WRITE);
  
  /*set type*/
  snd_seq_port_info_set_type(port_info_ptr, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
  
  /*set the port info in sequencer*/
  if (snd_seq_set_port_info(alsa_seq_client_handle, port_id, port_info_ptr) > 0){
    printf("error: couldn't update port info of object being moved\n");
    return -1;
  }
  
  /*free port descriptor structure*/
  snd_seq_port_info_free(port_info_ptr);
    
  free(ret);

  /*recursively update names of member objects for signal paths*/
  if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(rtobj)){
    node_t *temp_node;

    for (temp_node = ((signal_path_t*)rtobj->imp_struct)->object_list;\
	   temp_node; \
	   temp_node = temp_node->next){

      rtobject_update_alsa_seq_port_name((rtobject_t*)temp_node->data);
	
    }

  }
       
  return 0;
}
