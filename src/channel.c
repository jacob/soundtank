/*
 *  signal path channel code
 *
 *Copyright Jacob Robbins 2003-2004
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

#include "include.h"



channel_t* channel_alloca(int channel_scope){
  channel_t* newchan;

  if (!(newchan = (channel_t*)malloc(sizeof(channel_t)))){
    return 0;
  }

  newchan->index=0;
  newchan->scope=channel_scope;
  newchan->connect=0;
  newchan->buffer_struct=0;
  newchan->reference_list=0;

  return newchan;
}


void channel_dealloca(channel_t* chan){
  
  while (chan->reference_list)  {
    free((int*)chan->reference_list->data);
    ll_remove(chan->reference_list,&chan->reference_list);
  }

  free(chan);

}

int channel_get_index(const channel_t* chan){
  return chan->index;
}

void channel_set_index(channel_t* chan, int newindex){
  chan->index = newindex;
}

int channel_get_scope(const channel_t* chan){
  return chan->scope;
}

buffer_t* channel_get_buffer(const channel_t* chan){
  return chan->buffer_struct;
}

int channel_set_buffer(channel_t* chan, buffer_t* buff, int chan_address){

  /*remove reference from old buffer if necessary*/
  if ((chan->buffer_struct)&&(buff != chan->buffer_struct))
    buffer_remove_reference(chan->buffer_struct, chan_address);

  /*add reference to new buffer*/
  if (buff){
    if ((buffer_add_reference(buff, chan_address, BUFF_SHARE)) < 0){
      printf("channel set buffer error: failed attempt to add reference to buffer\n");
      return -1;
    }
  }
 
  chan->buffer_struct = buff;

  return 0;
}

channel_t* channel_get_connected_to_channel(const channel_t* chan){
  return chan->connect;
}

int channel_connect_to_channel(channel_t* chan, int object_address, channel_t* to_chan){

  if ((chan->connect)&&(chan->connect != to_chan))
    channel_disconnect(chan, object_address);

  if (chan->connect != to_chan){
    if ((channel_add_reference(to_chan, object_address)) < 0){
      printf("channel connect error: couldn't add reference to to channel\n");
      return -1;
    }
  }

  if ((channel_set_buffer(chan,channel_get_buffer(to_chan),object_address)) < 0){
    printf("channel connect error: couldn't get new buffer to use\n");
    channel_remove_reference(to_chan, object_address);
    return -1;
  }
 
  chan->connect = to_chan;

  return 0;
}


int channel_disconnect(channel_t* chan, int object_address){

  if ((chan->connect)&&((channel_remove_reference(chan->connect,object_address)) < 0))
    printf("channel disconnect warning: couldn't disconnect from parent channel, data corrupted\n");

  return channel_set_buffer(chan,0,object_address);
}


int channel_add_reference(channel_t* chan, int object_address){
  int* address;
  address = (int*)malloc(sizeof(int));
  if (!address) return -1;
  (*address) = object_address;
  ll_append(&chan->reference_list,(void*)address);
  return 0;
}

int channel_remove_reference(channel_t* chan, int object_address){

  node_t* temp_node;
  if (!chan->reference_list) return -1;
  for (temp_node = chan->reference_list;temp_node;temp_node=temp_node->next){
    if ( *((int*)temp_node->data) == object_address){
      ll_remove(temp_node,&chan->reference_list);
      return 0;
    }
  }
  return -1;
}

int channel_check_reference(const channel_t* chan, int object_address){
  node_t* temp_node;
  if (!chan->reference_list) return 0;
  for (temp_node=chan->reference_list;temp_node;temp_node=temp_node->next)
    if (*((int*)temp_node->data) == object_address) return 1;

  return 0;
}

int channel_get_reference_count(const channel_t* chan){
  return ll_get_size(&chan->reference_list);
}
