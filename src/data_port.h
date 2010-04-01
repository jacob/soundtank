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

#ifndef SOUNDTANK_DATA_PORT_INCLUDE
#define SOUNDTANK_DATA_PORT_INCLUDE


typedef struct data_port data_port_t;
struct data_port{
  char description_family;
  char* data_port_description_string; /*null-terminated, will equate
					to LADSPA port name*/
  int data_port_description; /*will equate to LADSPA port range hint*/
  unsigned long ladspa_port;
  int label; /*TODO: remove this*/
  int buffer_type;
  buffer_t* buffer_struct;
  buffer_t* working_buffer; /*used for LADSPA run-adding emulation in
			      plugins w/o that facility*/
  unsigned input : 1;
  channel_t* channel;
  char* target_pathname;
  int target_port;
  
};

typedef buffer_t* data_port_instance_t;



data_port_t* data_port_alloc(int input);
void data_port_dealloc(data_port_t* dport);

void data_port_zero_out(data_port_t* dport);

const char* data_port_get_description_string(const data_port_t* dport);
int data_port_set_description_string(data_port_t* dport, const char *desc);

int data_port_get_input(const data_port_t* dport);
int data_port_get_scope(const data_port_t* dport);

buffer_t* data_port_get_buffer_struct(const data_port_t* dport);
int data_port_set_buffer_struct(data_port_t* dport, buffer_t* new_buff, int obj_address);

channel_t* data_port_get_channel(const data_port_t* dport);
int data_port_set_channel(data_port_t* dport, channel_t* new_chan, int obj_address);

const char* data_port_get_target_pathname(const data_port_t* dport);
int data_port_get_target_port(const data_port_t* dport);

int data_port_set_target(data_port_t* dport, const char* new_pathname, int target_port);

data_port_t* data_port_create_from_ladspa_port(const LADSPA_PortDescriptor * port,\
					       const char * const port_name,\
					       const LADSPA_PortRangeHint * port_range_hint,\
					       unsigned long port_index);



#endif
