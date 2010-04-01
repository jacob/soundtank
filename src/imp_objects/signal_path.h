/*
 *  signal path implementation object code
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


#ifndef IMP_OBJECT_SIGNAL_PATH_INCLUDE
#define IMP_OBJECT_SIGNAL_PATH_INCLUDE



struct signal_path_struct{

  int owner_object_address;

  ll_head channel_list;

  ll_head object_list;

  namespace_t *namespace;

};



/*WARNING: THESE FUNCTIONS ARE ONLY THREAD SAFE WITH REGARDS TO THE
  ENGINE'S REALTIME THREAD*/
/*THE FUNCTIONS IN THIS HEADER ARE NOT THREAD SAFE FOR CONCURRENT USEAGE*/
/*MULTIPLE UI THREADS MUST SERIALIZE ACCESS TO THESE FUNCTIONS*/





signal_path_t* imp_object_signal_path_alloca(int path_address);
void imp_object_signal_path_dealloca(signal_path_t* sigpath);

int create_imp_object_signal_path(rtobject_t* rtobj);
int destroy_imp_object_signal_path(rtobject_t* rtobj);

int init_instance_signal_path(rtobject_t* rtobj, rtobject_instance_t* rtins);
int deinit_instance_signal_path(rtobject_t* rtobj, rtobject_instance_t* rtins);

int signal_path_get_owner_rtobject_address(const signal_path_t* sigpath);
int signal_path_is_master_path(const signal_path_t* sigpath);
int signal_path_get_position_from_node(const signal_path_t* sigpath, const node_t* node); /*NOT ZERO BASED!*/

node_t* signal_path_get_node(const signal_path_t* sigpath, const rtobject_t* rtobj); 
node_t* signal_path_get_node_from_position(const signal_path_t* sigpath, int pos); /*NOT ZERO BASED!*/
node_t* signal_path_get_node_from_address(const signal_path_t* sigpath, int address);
rtobject_t* signal_path_get_rtobject_from_address(const signal_path_t* sigpath, int address);

node_t* signal_path_get_node_from_name(const signal_path_t* sigpath, const char* name);
rtobject_t* signal_path_get_rtobject_from_name(const signal_path_t* sigpath, const char* name);
int signal_path_get_unique_name(const signal_path_t* sigpath, const char* try_name, char** result);

channel_t* signal_path_get_channel(const signal_path_t* sigpath, int index, int scope);
channel_t* signal_path_get_local_channel(const signal_path_t* sigpath, int index);
channel_t* signal_path_get_shared_channel(const signal_path_t* sigpath, int index);
channel_t* get_global_channel(int index);

int signal_path_attach_channel(signal_path_t* sigpath, channel_t* chan);
int signal_path_detach_channel(signal_path_t* sigpath, channel_t* chan);

channel_t* signal_path_create_new_channel(signal_path_t* sigpath, int scope);
channel_t* signal_path_create_channel(signal_path_t* sigpath, int index, int scope);
int signal_path_destroy_channel(signal_path_t* sigpath, channel_t* chan);

int signal_path_get_exposed_port_count(const signal_path_t* sigpath);
data_port_t* signal_path_get_exposed_port(const signal_path_t* sigpath, int port_index);
rtobject_t* signal_path_get_exposed_port_owner_rtobject(signal_path_t* sigpath, int port_index);

int signal_path_cleanup(signal_path_t* sigpath);

int signal_path_get_instance_count(rtobject_t* rtobj);
int signal_path_fill_new_process_list(rtobject_t* rtobj, rtobject_instance_t** proclist, int curr_pos, ll_head* dirty_object_list);

/* (POS < 0) = APPEND, (POS == 0) = PREPEND, OTHERWISE INSERT AFTER ELEMENT POS, NOT ZERO BASED!*/ 
/*NB: caller must call update_process_list after calling insert, move
  or remove as these fxns do not do that. Otherwise, changes will not
  take effect for live instances and will only effect rtobjects.*/
int signal_path_insert(signal_path_t* sigpath, node_t* obj_node, int pos);
int signal_path_move(signal_path_t* sigpath, node_t* obj_node, int section_size, int pos);
int signal_path_remove(signal_path_t* sigpath, node_t* obj_node, int section_size, node_t** result);
int signal_path_inter_path_move(signal_path_t* path, node_t* obj_node, int section_size, \
				signal_path_t* topath, int pos);




#endif
