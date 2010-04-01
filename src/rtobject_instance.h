/*
 * rtobject instance code
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

#ifndef REALTIME_OBJECT_INSTANCE_INCLUDE
#define REALTIME_OBJECT_INSTANCE_INCLUDE

#include <jack/jack.h>


typedef union rtobject_instance_imp_data imp_data_t;
union rtobject_instance_imp_data{

  struct {
    int device_channel;
    void* device_buffer;
  }extern_out_element;
  
  struct {
    jack_port_t* jack_port;
  }jack_extern_element;

  struct {
    ll_head local_buffer_list;
  }signal_path_element;
  
  struct {
    int curr_pos;
  }test_source_element;
  
  struct {
    const LADSPA_Descriptor* descriptor;
    LADSPA_Handle handle;
    unsigned overwrite : 1;
    ll_head output_data_port_list;
  }ladspa_plugin_element;
  
};


struct realtime_object_instance{
  int imp_type;
  unsigned active_before : 1; 
  unsigned unmuted_before : 1;
  int data_port_list_size;
  data_port_instance_t* data_port_list;  
  int control_list_size;
  control_instance_t* control_list;
  imp_data_t imp_data;
  unsigned char note_flag; /*used to map midi notes to particular instances*/
  int process_index; /*TODO set this in remake process list*/
  int major_type; /*can we remove this?*/

};

/*NOTE ( active_before == true ) => instance was active during last
 process fxn. it is used in the process fxn to ramp in when an
 instance starts playing and ramp out when it stops. This prevents
 clicks caused by the signal abruptly jumping to zero. Same for
 unmuted_before vis a vis muting*/



rtobject_instance_t* rtobject_instance_alloca();
rtobject_instance_t* rtobject_instance_alloca_copy(const rtobject_instance_t* orig_ins);
void rtobject_instance_dealloca(rtobject_instance_t* ins);

int create_rtobject_instance(rtobject_t* rtobj);
int create_rtobject_instance_copy(rtobject_t* rtobj, rtobject_instance_t* orig_ins);
int destroy_rtobject_instance(rtobject_t* rtobj);

int rtobject_instance_attach_data_ports(rtobject_t* rtobj, rtobject_instance_t* rtins);
int rtobject_instance_detach_data_ports(rtobject_t* rtobj, rtobject_instance_t* rtins);





#endif
