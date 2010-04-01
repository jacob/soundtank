/*
 *soundtank - realtime samples, virtual-synths, effects, input maps, lists
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

#ifndef SOUNDTANK_SOUNDTANK_INCLUDE
#define SOUNDTANK_SOUNDTANK_INCLUDE


rtobject_t* get_rtobject_from_address(int address);
rtobject_t* get_rtobject_from_path(const char* name);

void release_address(int address);
int get_free_address();

void release_buffer(int object_address, buffer_t* buff);
buffer_t* get_free_buffer();
buffer_t* get_shared_buffer(int object_address);
buffer_t* get_working_buffer(int index);

int get_scale_list_size();
scale_t* get_scale(int pos);
scale_t* get_scale_by_name(const char *name);
int add_scale(scale_t* new_scale);
int remove_scale(int pos);
int remove_scale_by_name(const char *name);

int remake_process_list();

int pointer_hot_swap(void** old_ptr, void** new_ptr);


/*generic rtobject in rtobject.c*/

/*implementation objects in imp_objects directory*/

/*built-in user interaction commands in commands directory*/



/*the following function is actually in data_port.c*/
int connect_data_ports(rtobject_t* from, int from_port_index, rtobject_t* to, int to_port_index);


#endif



