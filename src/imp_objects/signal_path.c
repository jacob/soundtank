/*
 * signal path implementation object code
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
#include "../soundtank_structs.h"

/*WARNING: THESE FUNCTIONS ARE ONLY THREAD SAFE WITH REGARDS TO THE
  ENGINE'S REALTIME THREAD*/
/*THE FUNCTIONS IN THIS HEADER ARE NOT THREAD SAFE FOR CONCURRENT USEAGE*/
/*MULTIPLE UI THREADS MUST SERIALIZE ACCESS TO THESE FUNCTIONS*/

/*used by namespace to get name of an rtobject node*/
static const char* rtobject_name_lookup(node_t* lookup_node){
  return rtobject_get_name((rtobject_t*)lookup_node->data);
}


signal_path_t* imp_object_signal_path_alloca(int path_address){
  signal_path_t* newpath;
  newpath = (signal_path_t*)malloc(sizeof(signal_path_t));
  if (!newpath) return 0;
  newpath->owner_object_address=path_address;
  newpath->channel_list=0;
  newpath->object_list=0;

  /*create namespace struct*/
  if (!(newpath->namespace = create_namespace(&newpath->object_list, rtobject_name_lookup))){
    printf("memory error\n");
    free(newpath);
    return 0;
  }

  return newpath;
}


void imp_object_signal_path_dealloca(signal_path_t* sigpath){
  channel_t *curr_chan;

  while ((curr_chan = signal_path_get_channel(sigpath, 0, CHANNEL_SCOPE_LOCAL))){
    
    if (signal_path_destroy_channel(sigpath, curr_chan) < 0){
      printf("signal path deallocation error: could not destory channel\n");
      break;
    }

    channel_dealloca(curr_chan);

    ll_remove(sigpath->channel_list,&sigpath->channel_list);
  }

  while (sigpath->object_list){
    printf("WARNING: deleting signal path with members\n");
    ll_remove(sigpath->object_list,&sigpath->object_list);
  }
   
  destroy_namespace(sigpath->namespace);

  free(sigpath);
}

int create_imp_object_signal_path(rtobject_t* rtobj){
  signal_path_t* newpath;
  
  if (!(newpath = imp_object_signal_path_alloca(rtobject_get_address(rtobj)))){
    printf("error: failed to create imp obj signal path\n");
    return -1;
  }

  rtobj->imp_struct = (void*)newpath;

  return 0;
}

int destroy_imp_object_signal_path(rtobject_t* rtobj){

  imp_object_signal_path_dealloca((signal_path_t*)rtobj->imp_struct);
  rtobj->imp_struct=0;

  return 0;
}

int init_instance_signal_path(rtobject_t* rtobj, rtobject_instance_t* rtins){
  node_t* temp_node;
  channel_t* curr_chan;
  signal_path_t* sigpath = (signal_path_t*)rtobj->imp_struct;

  /*sanity check, make sure instance is new*/
  if (rtins->imp_data.signal_path_element.local_buffer_list){
    printf("signal path error: trying to initialize non-empty instance\n");
    return -1;
  }

  /*fill instance's local buffer list*/
  for (temp_node=sigpath->channel_list;temp_node;temp_node=temp_node->next){

    curr_chan = (channel_t*)temp_node->data;

    /*don't add shared or global channels to instance list*/
    /*the instance list is for zeroeing and only local channels are zeroed*/
    //if (CHANNEL_SCOPE_LOCAL == channel_get_scope(curr_chan)){
      ll_append(&rtins->imp_data.signal_path_element.local_buffer_list,\
		(void*)curr_chan->buffer_struct);
      // }

  }

  return 0;
}


int deinit_instance_signal_path(rtobject_t* rtobj, rtobject_instance_t* rtins){

  /*purge instance's local buffer list*/
  while (rtins->imp_data.signal_path_element.local_buffer_list){
    ll_remove(rtins->imp_data.signal_path_element.local_buffer_list,\
	      &rtins->imp_data.signal_path_element.local_buffer_list);
  }

  return 0;
}

int signal_path_get_owner_rtobject_address(const signal_path_t* sigpath){
  return sigpath->owner_object_address;
}

int signal_path_is_master_path(const signal_path_t* sigpath){
  /*there is a separate fxn for this in case the implementation changes*/
  if (sigpath == master_path) return 1;
  return 0;
}

int signal_path_get_position_from_node(const signal_path_t* sigpath, const node_t* node){
  node_t* temp_node;
  int i;

  i = 1; /*NOT ZERO BASED!!!*/
  for (temp_node=sigpath->object_list;temp_node;temp_node=temp_node->next){
    if (temp_node == node) return i;
    ++i; 
  }

  return -1;
}

node_t* signal_path_get_node(const signal_path_t* sigpath, const rtobject_t* rtobj){
  node_t* temp_node;
  node_t* other_node;
  rtobject_t* curr_obj;

  for (temp_node=sigpath->object_list;temp_node;temp_node=temp_node->next){

    curr_obj = (rtobject_t*)temp_node->data;

    if (rtobj == curr_obj) return temp_node;

    if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(curr_obj))
      if ((other_node = signal_path_get_node((signal_path_t*)curr_obj->imp_struct,rtobj)))
	return other_node;

  }

  return 0;
}

node_t* signal_path_get_node_from_position(const signal_path_t* sigpath, int pos){
  node_t* temp_node;
  int i;

  if (!sigpath->object_list) return 0;

  if (pos < 0){

    /*last node*/
    temp_node = sigpath->object_list;
    while (temp_node->next) temp_node=temp_node->next;

  }else if (pos == 0){

    printf("WARNING: YOU ARE PROBABLY MISTAKENLY THINKING THAT SIGPATH_GET_NODE IS ZERO BASED\n");
    temp_node = sigpath->object_list;

  }else{

    temp_node = sigpath->object_list;
    for (i=0;i<(pos-1);++i){
      if (!temp_node->next) break;
      temp_node=temp_node->next;
    }

  }

  return temp_node;
}

node_t* signal_path_get_node_from_address(const signal_path_t* sigpath, int address){
  node_t* temp_node;
  node_t* other_node;
  rtobject_t* curr_obj;

  for (temp_node=sigpath->object_list;temp_node;temp_node=temp_node->next){

    curr_obj = (rtobject_t*)temp_node->data;

    if (address == rtobject_get_address(curr_obj)) return temp_node;

    if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(curr_obj))
      if ((other_node = signal_path_get_node_from_address((signal_path_t*)curr_obj->imp_struct,address)))
	return other_node;

  }

  return 0;
}


rtobject_t* signal_path_get_rtobject_from_address(const signal_path_t* sigpath, int address){
  node_t* temp_node;
  if ((temp_node = signal_path_get_node_from_address(sigpath,address)))
    return (rtobject_t*)temp_node->data;
  return 0;
}



node_t* signal_path_get_node_from_name(const signal_path_t* sigpath, const char* name){
  return ns_search(sigpath->namespace, name, 0, SEARCH_EXACT);
}


rtobject_t* signal_path_get_rtobject_from_name(const signal_path_t* sigpath, const char* name){
  node_t* temp_node;
  if ((temp_node = signal_path_get_node_from_name(sigpath,name)))
    return (rtobject_t*)temp_node->data;
  return 0;
}

int signal_path_get_unique_name(const signal_path_t* sigpath, const char* try_name, char** result){
  if (!(*result = ns_make_unique_name(sigpath->namespace, try_name)))
    return -1;
  return 0;
}

channel_t* signal_path_get_channel(const signal_path_t* sigpath, int index, int scope){
  node_t* temp_node;
  channel_t* curr_chan;

  for (temp_node=sigpath->channel_list;temp_node;temp_node=temp_node->next){
    
    curr_chan = (channel_t*)temp_node->data;
    if ((index == channel_get_index(curr_chan))&&(scope == channel_get_scope(curr_chan))) 
      return curr_chan;

  }

  return 0;
}

channel_t* signal_path_get_local_channel(const signal_path_t* sigpath, int index){
  return signal_path_get_channel(sigpath, index, CHANNEL_SCOPE_LOCAL);
}

channel_t* signal_path_get_shared_channel(const signal_path_t* sigpath, int index){
  return signal_path_get_channel(sigpath, index, CHANNEL_SCOPE_SHARE);
}

channel_t* get_global_channel(int index){
  return signal_path_get_channel(master_path,index,CHANNEL_SCOPE_GLOBAL);
}

int signal_path_attach_channel(signal_path_t* sigpath, channel_t* chan){
  int sigpath_address;
  rtobject_t* sigpath_rtobj;
  signal_path_t* parent;

  sigpath_address = signal_path_get_owner_rtobject_address(sigpath);

  if (!(sigpath_rtobj = get_rtobject_from_address(sigpath_address))){
    printf("signal path attach channel error: couldn't find signal path's rtobject\n");
    return -1;
  }

  parent = rtobject_get_parent_path(sigpath_rtobj);

  if (!signal_path_is_master_path(sigpath)){
    /*child to parent call: */
    node_t* temp_node;
    channel_t* curr_chan;
    channel_t* att_chan = 0;

    /*use existing connection if there is one*/
    if (!(att_chan = channel_get_connected_to_channel(chan))){
  

      /*check for pre-existing parent channels to attach child to */
      for (temp_node=parent->channel_list;temp_node;temp_node=temp_node->next){
	curr_chan = (channel_t*)temp_node->data;
	if ((CHANNEL_SCOPE_SHARE == channel_get_scope(curr_chan))&&\
	    (!(channel_check_reference(curr_chan,sigpath_address)))){
	  att_chan = curr_chan;
	  break;
	}
      }
      
      /*if no pre-existing parent channel was found, create new parent channel to attach child to */
      if (!att_chan){
	if (signal_path_is_master_path(parent)){
	  if (!(att_chan = signal_path_create_new_channel(parent, CHANNEL_SCOPE_GLOBAL))){
	    printf("signal path error: failed attempt to create channel\n");
	    return -1;
	  }
	}else{
	  if (!(att_chan = signal_path_create_new_channel(parent, CHANNEL_SCOPE_LOCAL))){
	    printf("signal path error: failed attempt to create channel\n");
	    return -1;
	  }
	}
      }

    }

    /*attach child channel to parent channel*/
    if ((channel_connect_to_channel(chan, sigpath_address, att_chan)) < 0){
      printf("signal path error: failed attempt to connect channel to parent's channel\n");
      return -1;
    }

    
  }else{
    /*master path call*/
    buffer_t* new_buff;
  
    /*get buffer from app facility and attach to channel */
    if (!(new_buff = get_free_buffer())){
      printf("master path error: failed attempt to acquire new buffer from Soundtank\n");
      return -1;
    }

    /*attach channel to buffer*/
    channel_set_buffer(chan,new_buff,sigpath_address);
    
  }

  return 0;
}

int signal_path_detach_channel(signal_path_t* sigpath, channel_t* chan){
  int sigpath_address;
  rtobject_t* sigpath_rtobj;
  signal_path_t* parent;

  sigpath_address = signal_path_get_owner_rtobject_address(sigpath);

  if (!(sigpath_rtobj = get_rtobject_from_address(sigpath_address))){
    printf("signal path attach channel error: couldn't find signal path's rtobject\n");
    return -1;
  }

  parent = rtobject_get_parent_path(sigpath_rtobj);

  if (!signal_path_is_master_path(sigpath)){
    channel_t* parent_chan;

    parent_chan = channel_get_connected_to_channel(chan);

    /*remove reference from parent channel*/
    channel_disconnect(chan, sigpath_address);
    
    /*see if parent channel is free to be destroyed*/
    if ((parent_chan)&&(!(channel_get_reference_count(parent_chan)))) 
      signal_path_destroy_channel(parent,parent_chan);
  

  }else{

    /*TODO: FIX THIS TO FREE UNUSED BUFFERS*/

    /*just disconnect channel from buffer*/
    channel_set_buffer(chan, 0, sigpath_address);

  }

  return 0;
}

int signal_path_get_free_index(signal_path_t* sigpath, int scope){
  node_t* temp_node;
  channel_t* curr_chan;
  int max;
 
  max = 0;

  /*current imp: find max index and add 1*/
  for (temp_node=sigpath->channel_list;temp_node;temp_node=temp_node->next){
    curr_chan = (channel_t*)temp_node->data;
    if ((scope == channel_get_scope(curr_chan))&&(max < channel_get_index(curr_chan))) 
      max = channel_get_index(curr_chan);
  }
  
  return max +1;
}


channel_t* signal_path_create_new_channel(signal_path_t* sigpath, int scope){
  return signal_path_create_channel(sigpath, signal_path_get_free_index(sigpath, scope), scope);
}

channel_t* signal_path_create_channel(signal_path_t* sigpath, int index, int scope){
  rtobject_t* sigpath_rtobj;
  channel_t* new_chan;
  node_t* new_chan_node;

  sigpath_rtobj = get_rtobject_from_address(signal_path_get_owner_rtobject_address(sigpath));

  /*make sure meaningful index does not exist already*/
  if (signal_path_get_channel(sigpath, index, scope)){
    printf("signal path error: trying to create channel that already exists\n");
    return 0;
  }

  /*create new channel*/
  if (!(new_chan = channel_alloca(scope))){
    printf("signal path error: memory error\n");
    return 0;
  }

  /*set new channel's index*/
  channel_set_index(new_chan,index);

  /*add new channel to channel list*/
  if (!(new_chan_node = ll_append(&sigpath->channel_list, (void*)new_chan))){
    printf("signal path error: linked list append error\n");
    channel_dealloca(new_chan);
    return 0;
  }

  /*attach new channel*/
  if ((signal_path_attach_channel(sigpath, new_chan )) < 0){
    printf("signal path error: failed attempt to attach channel\n");
    ll_remove(new_chan_node, &sigpath->channel_list);
    free(new_chan_node);
    channel_dealloca(new_chan);
    return 0;
  }

  /*update instance situation*/
  rtobject_update_instance_situation(sigpath_rtobj);

  return new_chan;
}

int signal_path_destroy_channel(signal_path_t* sigpath, channel_t* chan){
  rtobject_t* sigpath_rtobj;
 
  /*sanity check: verify signal path rtobject is available (for changing rtobject_instance)*/
  if (!(sigpath_rtobj = get_rtobject_from_address(signal_path_get_owner_rtobject_address(sigpath)))){ 
    printf("signal path error: could not find signal path rtobject\n");
    return -1;
  }
  
  /*sanity check: make sure context element has no references*/
  if ((channel_get_reference_count(chan)) > 0){
    printf("signal path error: trying to destroy channel that has an active reference\n");
    return -1;
  }
  
  /*detach channel*/
  if ((signal_path_detach_channel(sigpath,chan)) < 0){
    printf("signal path destroy channel error: failed attempt to detach from parent channel\n");
    return -1;
  }
    
  /*update instance situation*/
  rtobject_update_instance_situation(sigpath_rtobj);
  
  return 0;
}

int signal_path_get_exposed_port_count(const signal_path_t* sigpath){
  /*count all local ins' inputs and local outs' outputs, these ports
    are exposed by the signal path as if they were its own*/
  const node_t *temp_node;
  const rtobject_t* curr_rtobj;
  const data_port_t* curr_port;
  int i,count;

  count=0;

  for (temp_node=sigpath->object_list;temp_node;temp_node=temp_node->next){
    curr_rtobj = (rtobject_t*)temp_node->data;
    
    if ((RTOBJECT_MAJOR_TYPE_LOCAL_OUT == rtobject_get_major_type(curr_rtobj))){
      for (i=0;i<rtobject_get_data_port_list_size(curr_rtobj);++i){
	curr_port = rtobject_get_data_port(curr_rtobj,i);
	if (!(data_port_get_input(curr_port))) ++count;
      }
    }

    if ((RTOBJECT_MAJOR_TYPE_LOCAL_IN == rtobject_get_major_type(curr_rtobj))\
	||(RTOBJECT_MAJOR_TYPE_EXTERN_OUT == rtobject_get_major_type(curr_rtobj))){
      for (i=0;i<rtobject_get_data_port_list_size(curr_rtobj);++i){
	curr_port = rtobject_get_data_port(curr_rtobj,i);
	if ((data_port_get_input(curr_port))) ++count;
      }
    }

  }

  return count;
}

data_port_t* signal_path_get_exposed_port(const signal_path_t* sigpath, int port_index){
  /*count all local ins' inputs and local outs' outputs, these ports
    are exposed by the signal path as if they were its own*/
  const node_t *temp_node;
  const rtobject_t* curr_rtobj;
  data_port_t* curr_port;
  int i,count;

  count=0;

  for (temp_node=sigpath->object_list;temp_node;temp_node=temp_node->next){
    curr_rtobj = (rtobject_t*)temp_node->data;
    
    if ((RTOBJECT_MAJOR_TYPE_LOCAL_OUT == rtobject_get_major_type(curr_rtobj))){
      for (i=0;i<rtobject_get_data_port_list_size(curr_rtobj);++i){
	curr_port = rtobject_get_data_port(curr_rtobj,i);
	if (!(data_port_get_input(curr_port))) 
	  if (port_index == count++) return curr_port;
      }
    }

    if ((RTOBJECT_MAJOR_TYPE_LOCAL_IN == rtobject_get_major_type(curr_rtobj))\
	||(RTOBJECT_MAJOR_TYPE_EXTERN_OUT == rtobject_get_major_type(curr_rtobj))){
      for (i=0;i<rtobject_get_data_port_list_size(curr_rtobj);++i){
	curr_port = rtobject_get_data_port(curr_rtobj,i);
	if ((data_port_get_input(curr_port)))
 	  if (port_index == count++) return curr_port;
     }
    }

  }

  return 0;
}

rtobject_t* signal_path_get_exposed_port_owner_rtobject(signal_path_t* sigpath, int port_index){
  /*count all local ins' inputs and local outs' outputs, these ports
    are exposed by the signal path as if they were its own*/
  node_t *temp_node;
  rtobject_t* curr_rtobj;
  data_port_t* curr_port;
  int i,count;

  count=0;

  for (temp_node=sigpath->object_list;temp_node;temp_node=temp_node->next){
    curr_rtobj = (rtobject_t*)temp_node->data;
    
    if ((RTOBJECT_MAJOR_TYPE_LOCAL_OUT == rtobject_get_major_type(curr_rtobj))){
      for (i=0;i<rtobject_get_data_port_list_size(curr_rtobj);++i){
	curr_port = rtobject_get_data_port(curr_rtobj,i);
	if (!(data_port_get_input(curr_port))) 
	  if (port_index == count++) return curr_rtobj;
      }
    }

    if ((RTOBJECT_MAJOR_TYPE_LOCAL_IN == rtobject_get_major_type(curr_rtobj))\
	||(RTOBJECT_MAJOR_TYPE_EXTERN_OUT == rtobject_get_major_type(curr_rtobj))){
      for (i=0;i<rtobject_get_data_port_list_size(curr_rtobj);++i){
	curr_port = rtobject_get_data_port(curr_rtobj,i);
	if ((data_port_get_input(curr_port)))
 	  if (port_index == count++) return curr_rtobj;
     }
    }

  }

  return 0;
}


/*******************adding/removing context buffers to a live signal path*****
**** 
****  --  Upon allocation/deallocation of a context buffer, 
****    signal_path_update_instance_situation() fxn is called so that
****    a _new_ instance is created with a _new_ copy of the buffer list.
****    The new instance is put into the rtobject instance list and the old instance
****    is put onto the outdated pointer in the rtobject.
****    When the signal path has its process list remade, 
****    the new instance will be swapped in automatically
****
****  --  In the remake_process_list fxn, the signal_path_cleanup() fxn is called
****    so that the outdated instance is deinited & freed, ports are dereferenced,
****    and buffers are released.
****
****
****
****   If you can think of a simpler way to achieve this, please tell me:
****
****                       jrobbins @ newyorkmusicunion . com
****
******************************************************************************/



int signal_path_cleanup(signal_path_t* sigpath){
  node_t* temp_node;
  channel_t* curr_chan;

  if (debug_readout) printf("in cleanup fxn for %d\n",sigpath->owner_object_address);

  /*NOTE: This fxn should only be called from remake_process_list. The
        reason for this is that removing the "unused" buffers must be
        synched with the rt thread. Even when a context buffer is
        totally detached, there is still an instance using it until
        the active process list is swapped to remove that instance*/
  


  /*free unused channels*/
  for (temp_node=sigpath->channel_list;temp_node;temp_node=temp_node->next){
    curr_chan = (channel_t*)temp_node->data;

    if (!channel_get_reference_count(curr_chan)){
      
      temp_node = temp_node->next;

      signal_path_destroy_channel(sigpath,curr_chan);

      if (!temp_node) break;
      
    }
  }

  return 0;
}

int signal_path_get_instance_count(rtobject_t* rtobj){
  node_t* temp_node;
  rtobject_t* curr_obj;
  signal_path_t* sigpath;
  int count;

  if (rtobject_get_major_type(rtobj) != RTOBJECT_MAJOR_TYPE_SIGNAL_PATH){
    /*this is really a fatal error, but it should never happen*/
    printf("sigpath get instance count error: counting a non-path rtobject, ");
    printf("DATA CORRUPTION HAS OCCURED!\n");
    return 0;
  }

  count = 0;
  sigpath = (signal_path_t*)rtobj->imp_struct;

  /*count the instances that this signal path uses*/
  count += ll_get_size(&rtobj->instance_list);

  /*count the instances used by this signal path's member objects*/
  for (temp_node=sigpath->object_list;temp_node;temp_node=temp_node->next){

    curr_obj = (rtobject_t*)temp_node->data;

    if (rtobject_get_major_type(curr_obj) == RTOBJECT_MAJOR_TYPE_SIGNAL_PATH)
      count += signal_path_get_instance_count(curr_obj);
    else
      count += ll_get_size(&curr_obj->instance_list);

  }

  return count;
}


int signal_path_fill_new_process_list(rtobject_t* rtobj, \
				      rtobject_instance_t** proclist,\
				      int curr_pos,\
				      ll_head* dirty_object_list){
  node_t* temp_node;
  node_t* other_node;
  rtobject_t* curr_obj;
  signal_path_t* sigpath;
  int pos;

  if (rtobject_get_major_type(rtobj) != RTOBJECT_MAJOR_TYPE_SIGNAL_PATH){
    /*this is really a fatal error, and it should never happen*/
    printf("signal path fill process list error: filling a non-path rtobject, ");
    printf("DATA CORRUPTION HAS OCCURED!\n");
    return 0;
  }

  pos = curr_pos;
  sigpath = (signal_path_t*)rtobj->imp_struct;

  /*add signal path's instances to process list and record their process indexes*/
  for (temp_node=rtobj->instance_list;temp_node;temp_node=temp_node->next){
    ((rtobject_instance_t*)temp_node->data)->process_index = pos;
    proclist[pos++] = (rtobject_instance_t*)temp_node->data;
  }

  /*record dirty objects for cleaning*/
  if (rtobj->outdated_instance_list){
    if (!(ll_append(dirty_object_list, (void*)rtobj))){
      printf("warning: memory error while recording dirty objects, ");
      printf("DATA CORRUPTION HAS OCCURED!\n");
    }
  }
  
  for (temp_node=sigpath->object_list;temp_node;temp_node=temp_node->next){
    
    curr_obj = (rtobject_t*)temp_node->data;

    if (rtobject_get_major_type(curr_obj) == RTOBJECT_MAJOR_TYPE_SIGNAL_PATH)
      pos += signal_path_fill_new_process_list(curr_obj, proclist, pos, dirty_object_list);
    else{
      
      /*add rtobject's instances to process list and record their process indexes*/
      for (other_node=curr_obj->instance_list;other_node;other_node=other_node->next){
	((rtobject_instance_t*)other_node->data)->process_index = pos;
	proclist[pos++] = (rtobject_instance_t*)other_node->data;
      }
 
      /*record dirty objects for cleaning*/
      if (curr_obj->outdated_instance_list){
	if (!(ll_append(dirty_object_list, (void*)curr_obj))){
	  printf("warning: memory error while recording dirty objects, data corruption\n");
	}
      }
         
    }
   
  }

  return (pos - curr_pos);
}



int signal_path_insert(signal_path_t* sigpath, node_t* obj_node, int pos){
  node_t *temp_node, *obj_list_copy;
  int i;

  /* (POS < 0) = APPEND, (POS == 0) = PREPEND, OTHERWISE INSERT AFTER
     ELEMENT POS, NOT ZERO BASED!*/ 



  /*copy list of rtobjects to use later*/
  obj_list_copy = 0;
  if (ll_copy(&obj_list_copy, &obj_node) < 0){
    printf("signal path insert error: couldn't copy rtobject list\n");
    return -1;
  }

  /*make objects' names unique in object list*/
  for (temp_node=obj_list_copy;temp_node;temp_node=temp_node->next){
    const char* curr_name;
    rtobject_t* curr_obj;
    char* new_name;

    curr_obj = (rtobject_t*)temp_node->data;
    curr_name = rtobject_get_name(curr_obj);
    if ((signal_path_get_unique_name(sigpath, curr_name, &new_name))<0){
      printf("signal path insert error: failed attempt to find unique name\n");
      return -1;
    }
    if (strcmp(curr_name,new_name)){
      if ((rtobject_set_name(curr_obj,new_name))<0){
	printf("signal path insert error: failed attempt to change objects name\n");
	return -1;
      }
    }
    free(new_name);
  }

  /*set objects' parent pointers*/
  for (temp_node=obj_node;temp_node;temp_node=temp_node->next)
    ((rtobject_t*)temp_node->data)->parent = sigpath;

  /*add objects to object list*/
  if (pos < 0){
    /*append*/
    ll_section_paste_append(obj_node, &sigpath->object_list);

  }else if (pos == 0){
    /*prepend*/
    ll_section_paste_prepend(obj_node, &sigpath->object_list);

  }else{
    /*insert after element pos*/
    temp_node = sigpath->object_list;
    for (i=0;i<(pos-1);++i){
      if (!temp_node->next) break;
      temp_node=temp_node->next;
    }
    /*check for empty list case*/
    if (temp_node)
      ll_section_paste_after(temp_node, obj_node);
    else
      ll_section_paste_append(obj_node, &sigpath->object_list);
  }


  /*update object's ALSA sequencer port name*/
  for (temp_node=obj_list_copy; temp_node; temp_node=temp_node->next){
    
    if (rtobject_update_alsa_seq_port_name((rtobject_t*)temp_node->data) < 0){
      printf("signal path insert error: couldn't update ALSA port for ");
      printf("rtobject %s\n", rtobject_get_name((rtobject_t*)temp_node->data));
    }

  }

  /*remake process list*/
  /*letting caller take care of this
  if ((remake_process_list()) < 0){
    printf("error occured while remaking process list\n");
    return -1;
  }
  */
  
  return 0;
}

int signal_path_move(signal_path_t* sigpath, node_t* obj_node,\
		     int section_size, int pos){
  node_t* temp_node;

  /*move objects in object list*/
  if (pos > 0){
    node_t* other_node;
    other_node = signal_path_get_node_from_position(sigpath,pos);
    temp_node = ll_section_cut(obj_node, section_size, &sigpath->object_list);
    ll_section_paste_after(other_node, temp_node);
  }else if (pos == 0){
    /*prepend case*/
    temp_node = ll_section_cut(obj_node, section_size, &sigpath->object_list);
    ll_section_paste_prepend(temp_node,&sigpath->object_list);
  }else{
    /*append case*/
    temp_node = ll_section_cut(obj_node, section_size, &sigpath->object_list);
    ll_section_paste_append(temp_node,&sigpath->object_list);
  }

  /*remake process list*/
  /*letting caller take care of this
  if ((remake_process_list()) < 0){
    printf("error occured while remaking process list\n");
    return -1;
  }
  */

  return 0;
}

int signal_path_remove(signal_path_t* sigpath, node_t* obj_node,\
		       int section_size, node_t** result){
  node_t* temp_node;
  node_t* other_node;

  /*remove objects from object list*/
  temp_node = ll_section_cut(obj_node, section_size, &sigpath->object_list);

  /*unset objects' parent pointers*/
  for (other_node=temp_node;other_node;other_node=other_node->next)
    ((rtobject_t*)other_node->data)->parent = 0;

  /*set result pointer*/
  *result = temp_node;

  /*remake process list*/
  /*letting caller take care of this
  if ((remake_process_list()) < 0){
    printf("error occured while remaking process list\n");
    return -1;
  }
  */

  return 0;
}

int signal_path_inter_path_move(signal_path_t* path, node_t* obj_node,\
				int section_size, \
				signal_path_t* topath, int pos){
  int ret;
  node_t* mv_node;
  mv_node = 0;

  /*TODO : RECORD CURRENT POSITION OF OBJECTS FOR FAILURE ON REINSERTION*/

  ret = signal_path_remove(path, obj_node, section_size, &mv_node);

  if (ret < 0){
    printf("inter path move error: failed attempt to remove rtobject(s) from path\n");
    return -1;
  }

  ret = signal_path_insert(topath, mv_node, pos);

  if (ret < 0){
    printf("inter path move error: failed attempt to insert rtobject(s) into target path\n");
    if ((signal_path_insert(path, mv_node, pos)) < 0)
      printf("inter path move error: failed attempt to reinsert rtobject(s) into original path\n");
    return -1;
  }

  return 0;
}
