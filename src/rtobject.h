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

#ifndef REALTIME_OBJECT_INCLUDE
#define REALTIME_OBJECT_INCLUDE


/*TODO: move this forward reference elsewhere*/
typedef struct scale scale_t;


/*the following are some forward references to sub objects*/

/*from rtobject_instance.h*/
typedef struct realtime_object rtobject_t;
typedef struct realtime_object_instance rtobject_instance_t;

/*from event_map/event_map.h*/
typedef struct event_map event_map_t;

/*from imp_objects/signal_path.h*/
typedef struct signal_path_struct signal_path_t;



/*here is _the_ rtobject structure*/

struct realtime_object {

  /*core data*/
  char* name;
  char* description;
  int address;
  signal_path_t* parent;
  int process_index;
  int major_type;
  int imp_type; /*imp abbreviates implementation*/
  int imp_subtype; /*unused*/
  int imp_arg_list_size; 
  char** imp_arg_list; 
  void* imp_struct;

  /*controls*/
  ll_head control_list;
  namespace_t *control_ns;

  /*data ports*/
  int data_port_list_size;
  ll_head data_port_list;
  namespace_t *data_port_ns;

  /*instances*/
  int instance_list_size;
  ll_head instance_list;
  ll_head outdated_instance_list; /*used to cleanup after changing channels*/
 
  /*event maps*/
  generic_array_t* event_map_list;
    
};



rtobject_t* rtobject_alloca();
void rtobject_dealloca(rtobject_t* rtobj);

int create_rtobject(int major_type, int imp_type, int imp_argc, const char** imp_argv, char* name, char* description, signal_path_t* target_path);
int create_rtobject_dup(rtobject_t* old_obj, char* name, signal_path_t* target_path);
void destroy_rtobject(rtobject_t* rtobj);

const char* rtobject_get_name(rtobject_t* rtobj);
int rtobject_set_name(rtobject_t* rtobj,const char* new_name);

const char* rtobject_get_description(rtobject_t* rtobj);
int rtobject_set_description(rtobject_t* rtobj,const char* new_description);

int rtobject_get_address(const rtobject_t* rtobj);
int rtobject_get_process_index(const rtobject_t* rtobj);

rtobject_t* rtobject_get_parent(const rtobject_t* rtobj);
signal_path_t* rtobject_get_parent_path(const rtobject_t* rtobj);


int rtobject_get_major_type(const rtobject_t* rtobj);
int rtobject_get_implementation_type(const rtobject_t* rtobj);

int rtobject_get_implementation_arg_list_size(const rtobject_t* rtobj);
const char* rtobject_get_implementation_arg(const rtobject_t* rtobj,\
					    int whicharg);
int rtobject_get_control_list_size(const rtobject_t* rtobj);
const control_t* rtobject_get_control(const rtobject_t* rtobj, int pos);
int rtobject_get_control_index(const rtobject_t* rtobj, const char* name);
/*search control names, returns number of matches. must pass it an
  empty linked list head, each node returned has an allocated integer
  giving the control index. use free_search_results to deallocate the
  list.*/
int rtobject_search_controls(const rtobject_t* rtobj,\
			     const char *search_pattern,\
			     int search_type, ll_head* results);
void rtobject_free_search_results(ll_head* results);

int rtobject_get_data_port_list_size(const rtobject_t* rtobj);
data_port_t* rtobject_get_data_port(const rtobject_t* rtobj, int port_index);
rtobject_t* rtobject_get_data_port_owner_rtobject(rtobject_t* rtobj, int port_index); /*only necessary for signal paths*/
/*search data port names, returns number of matches. must pass it an
  empty linked list head, each node returned has an allocated integer
  giving the port index. use free_search_results to deallocate the
  list.*/
int rtobject_get_data_port_index(const rtobject_t* rtobj, const char *name);
int rtobject_search_data_ports(const rtobject_t* rtobj,\
			     const char *search_pattern,\
			     int search_type, ll_head* results);
/*please use: rtobject_free_search_results(ll_head* results);*/

int rtobject_attach_port_to_channel(rtobject_t* rtobj,\
				    int port_index, channel_t* chan);
int rtobject_detach_port(rtobject_t* rtobj, int port_index);

int rtobject_get_instance_list_size(const rtobject_t* rtobj);
rtobject_instance_t* rtobject_get_instance(const rtobject_t* rtobj, int pos);

int rtobject_update_instance_situation(rtobject_t* rtobj);
int rtobject_cleanup(rtobject_t* rtobj);

char* rtobject_get_absolute_pathname(rtobject_t* rtobj);
int rtobject_create_alsa_seq_port(rtobject_t* rtobj);
int rtobject_update_alsa_seq_port_name(rtobject_t* rtobj);


#endif
