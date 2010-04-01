/*
 * data port code
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
#include "soundtank_structs.h"


data_port_t* data_port_alloc(int input){
  enum libcousin_data_port_description data_port_desc;
  data_port_t* dport;

  if (!(dport = (data_port_t*)malloc(sizeof(data_port_t)))){
    return 0;
  }

  dport->description_family = DESCRIPTION_FAMILY_LIBCOUSIN;
  dport->data_port_description_string = 0;
  data_port_desc = mono;
  dport->data_port_description = (int)data_port_desc;
  dport->ladspa_port = -1;
  dport->label = -1;
  dport->buffer_type = BUFFER_TYPE_MONO;
  dport->buffer_struct = 0;
  dport->working_buffer = 0;
  if (input)
    dport->input = 1;  
  else
    dport->input = 0;
  dport->channel = 0;    
  dport->target_pathname = 0;
  dport->target_port = -1;

  
  return dport;
}

void data_port_dealloc(data_port_t* dport){

  if (dport->data_port_description_string)
   free(dport->data_port_description_string);

 free(dport);
}

void data_port_zero_out(data_port_t* dport){

  memset((void*)dport,0,sizeof(data_port_t));

  dport->target_port = -1;
}


const char* data_port_get_description_string(const data_port_t* dport){
  return dport->data_port_description_string;
}

int data_port_set_description_string(data_port_t* dport, const char *desc){
  if (dport->data_port_description_string)
    free(dport->data_port_description_string);

  if (!(dport->data_port_description_string = strdup(desc)))
    return -1;

  return 0;
}

int data_port_get_input(const data_port_t* dport){
  return dport->input;
}

int data_port_get_scope(const data_port_t* dport){
  channel_t* chan = data_port_get_channel(dport);
  if (chan)
    return channel_get_scope(chan);
  return -1;
}

buffer_t* data_port_get_buffer_struct(const data_port_t* dport){
  return dport->buffer_struct;
}

int data_port_set_buffer_struct(data_port_t* dport, buffer_t* buff, int obj_address){

  /*remove reference from old buffer if necessary*/
  if ((dport->buffer_struct)&&(buff != dport->buffer_struct))
    buffer_remove_reference(dport->buffer_struct, obj_address);

  /*add reference to new buffer*/
  if (buff){
    if ((buffer_add_reference(buff, obj_address, BUFF_SHARE)) < 0){
      printf("data port set buffer error: failed attempt to add reference to buffer\n");
      return -1;
    }
  }
 
  dport->buffer_struct = buff;

  return 0;
}

channel_t* data_port_get_channel(const data_port_t* dport){
  if ((dport->channel == null_read)||(dport->channel == null_write)) return 0;
  return dport->channel;
}


int data_port_set_channel(data_port_t* dport, channel_t* new_chan, int obj_address){

  /*remove reference from old channel if necessary*/
  if ((dport->channel)&&(new_chan != dport->channel)){
    if ((channel_remove_reference(dport->channel, obj_address)) < 0)
      printf("data port set channel warning: failed to detach from old channel data corruption\n");
  }

  if (new_chan){

    /*add reference to new channel*/
    if ((channel_add_reference(new_chan, obj_address)) < 0){
      printf("data port set channel error: failed attempt to add reference to new channel\n");
      return -1;
    }

    /*attach port channel's buffer*/
    if ((data_port_set_buffer_struct(dport, channel_get_buffer(new_chan), obj_address)) < 0){
      printf("data port set channel error: failed attempt to connect to new buffer\n");
      return -1;
    }

  }else{

    /*TODO : remove this*/
    printf("ERROR: CHANNEL = 0 NOT CORRECT IMP\n");
    return -1;

  }

  dport->channel = new_chan;

  return 0;
}

const char* data_port_get_target_pathname(const data_port_t* dport){
  if (dport->target_pathname) return dport->target_pathname;
  return "";
}

int data_port_get_target_port(const data_port_t* dport){
  return dport->target_port;
}

int data_port_set_target(data_port_t* dport, const char* new_pathname, int new_port){
 
  if (dport->target_port >= 0)  free(dport->target_pathname);

  if (new_port >= 0){
    if (!(dport->target_pathname = strdup(new_pathname))){
      printf("data port set target error: memory error\n");
      return -1;
    }
  }

  dport->target_port = new_port;

  return 0;
}

data_port_t* data_port_create_from_ladspa_port(const LADSPA_PortDescriptor * port,\
					       const char * const port_name,\
					       const LADSPA_PortRangeHint * port_range_hint,\
					       unsigned long port_index){

  data_port_t* new_port;

  if (!(new_port = data_port_alloc(LADSPA_IS_PORT_INPUT(*port))))
    return 0;

  if (!(new_port->data_port_description_string = strdup(port_name))){
    data_port_dealloc(new_port);
    return 0;
  }
   
  new_port->data_port_description = port_range_hint->HintDescriptor;
  new_port->ladspa_port = port_index;

  return new_port;
}







int connect_data_ports(rtobject_t* from, int from_port_index, rtobject_t* to, int to_port_index){
  data_port_t* from_port;
  data_port_t* to_port;
  channel_t* from_port_chan;
  channel_t* to_port_chan;
  channel_t* connection_chan;

  connection_chan = 0;

  /*Note to reader: I don't know where to put this function, it's a
    little to involved to really belong here. And too complicated for
    the att command. Suggestions are welcome This attaching stuff was
    unevenly spread out with a lot in sigpath that is now gone.*/
 
  /*XXX TODO: verify that from object is before to object in process order*/

  /*get from port, make sure it's output*/
  if (!(from_port = rtobject_get_data_port(from, from_port_index))){
    printf("connect ports error: connect-from port doesn't exist\n");
    return -1;
  }
 
  /*get from port channel, if it exists*/
  from_port_chan = data_port_get_channel(from_port); 

  /*check for easy case: to is parent path containing from*/
  /*in this case there is no to port, the to_port_index refers to the
    parent channel index*/
  if (to == rtobject_get_parent(from)){
    signal_path_t* parent_path;
    parent_path = rtobject_get_parent_path(from);
    
    /*does the local channel exist already? if not, then make it*/
    if (!(connection_chan =  signal_path_get_channel(parent_path,to_port_index, \
						     CHANNEL_SCOPE_LOCAL))){

      if (!(connection_chan = signal_path_create_channel(parent_path, to_port_index, \
							 CHANNEL_SCOPE_LOCAL))){
	printf("connect ports error: failed attempt to create local channel %d\n",to_port_index);
	return -1;
      }
       
    }

    /*attach from port to local channel if necessary*/
    if (from_port_chan != connection_chan){

      if ((rtobject_attach_port_to_channel(from, from_port_index, connection_chan)) < 0){
	printf("connect ports error: failed attempt to attach from-port to channel\n");
	return -1;
      }

      /*store target info in from port*/
      if ((data_port_set_target(from_port,"*",to_port_index)) < 0){
	printf("connect ports error: memory error\n");
	return -1;
      }

    }

    /*in this case, there's no target port, just a channel, so we're done*/
    
    return 0;
  }

  /*the remaining cases involve two ports (as opposed to a port and
    parent channel)*/

  /*if we are connecting two ports, the from port must be output & the
    to port input*/
  if (data_port_get_input(from_port)){
    printf("connect ports error: connect-from port is an input port\n");
    return -1;
  }

  /*get to port, make sure it's input*/
  if (!(to_port = rtobject_get_data_port(to, to_port_index))){
    printf("connect global error: connect-to port doesn't exist\n");
    return -1;
  }
  if (!(data_port_get_input(to_port))){
    printf("connect ports error: connect-to port is an output port\n");
    return -1;
  }
    
  /*get to port channel, if it exists*/
  to_port_chan = data_port_get_channel(to_port);  


  /*check for impossible case: to_port on a channel outside scope of
    from_port*/
  if ((to_port_chan)&&(rtobject_get_parent_path(from) != rtobject_get_parent_path(to))&&\
      (CHANNEL_SCOPE_GLOBAL != channel_get_scope(to_port_chan))){
    printf("connect ports error: connect-to-port is connected to an unreachable channel\n");
    printf("In this version of Soundtank that is not handled\n");
    /*the future fix is to create a local input right before to_port
      and attach to that*/
    return -1;
  }

  /*check for easy case: ports on same scope & local channel exists*/
  if (rtobject_get_parent_path(from) == rtobject_get_parent_path(to)){

    if (to_port_chan) connection_chan = to_port_chan;
    else if (from_port_chan) connection_chan = from_port_chan;

  }

  /*check if to_port is connected to a global channel*/
  if ((to_port_chan)&&\
      (CHANNEL_SCOPE_GLOBAL == channel_get_scope(to_port_chan))){
    connection_chan = to_port_chan;
  }

  /*if neither port is attached to any channel then we can create one*/
  if (!connection_chan){
    
    if (rtobject_get_parent_path(from) != rtobject_get_parent_path(to)){
      /*need a global scope channel*/
      
      if (!(connection_chan = signal_path_create_new_channel(master_path,CHANNEL_SCOPE_GLOBAL))){
	printf("connect ports error: failed attempt to make new channel\n");
	return -1;
      }
      
    }else{
      /*local scope channel will suffice*/ 
      
      if (!(connection_chan = signal_path_create_new_channel(rtobject_get_parent_path(to),CHANNEL_SCOPE_LOCAL))){
	printf("connect ports error: failed attempt to make new channel\n");
	return -1;
      }
      
    }
    
  }

  
  /*done finding channel to use, now attach ports to it*/

  /*attach target port to connection channel if necessary*/
  if (to_port_chan != connection_chan){

    if ((rtobject_attach_port_to_channel(to, to_port_index, connection_chan)) < 0){
      printf("connect ports error: failed attempt to attach to-port to new channel\n");
      return -1;
    } 

  }

  /*attach from port to connection channel if necessary*/
  if (from_port_chan != connection_chan){
    char* to_absolute_pathname;

    if ((rtobject_attach_port_to_channel(from, from_port_index, connection_chan)) < 0){
      printf("connect ports error: failed attempt to attach from-port to channel\n");
      return -1;
    }      

    /*store target info in from port*/
    to_absolute_pathname = rtobject_get_absolute_pathname(to);
    if ((data_port_set_target(from_port,to_absolute_pathname,to_port_index)) < 0){
      printf("connect ports error: memory error\n");
      free(to_absolute_pathname);
      return -1;
    }
    free(to_absolute_pathname);
  }
  
  /*NOTE: calling fxn is responsible for calling remake_process_fxn*/
  /*NOTE: this is so multiple ports can be attached before remaking proclist*/

  return 0;
}
