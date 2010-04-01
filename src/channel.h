/*
 *  signal path channel code
 *
 *Copyright Jacob Robbins 2003
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

#ifndef SIGNAL_PATH_CHANNEL_INCLUDE
#define SIGNAL_PATH_CHANNEL_INCLUDE



#define CHANNEL_SCOPE_LOCAL 0
#define CHANNEL_SCOPE_SHARE 1
#define CHANNEL_SCOPE_GLOBAL 2


typedef struct signal_path_channel channel_t;
struct signal_path_channel{
  int index;
  int scope;
  channel_t* connect;
  buffer_t* buffer_struct;
  ll_head reference_list;
};


channel_t* channel_alloca(int channel_scope);
void channel_dealloca(channel_t* chan);

int channel_get_index(const channel_t* chan);
void channel_set_index(channel_t* chan, int newindex);

int channel_get_scope(const channel_t* chan);

buffer_t* channel_get_buffer(const channel_t* chan);
int channel_set_buffer(channel_t* chan, buffer_t* buff, int chan_address);

channel_t* channel_get_connected_to_channel(const channel_t* chan);
int channel_connect_to_channel(channel_t* chan, int object_address, channel_t* to_chan);
int channel_disconnect(channel_t* chan, int object_address);


int channel_add_reference(channel_t* chan, int object_address);
int channel_remove_reference(channel_t* chan, int object_address);
int channel_check_reference(const channel_t* chan, int object_address);
int channel_get_reference_count(const channel_t* chan);

#endif
