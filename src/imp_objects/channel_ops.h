/*
 *soundtank - channel operations implementation objects code
 *
 *Copyright Jacob Robbins 2004
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

#ifndef SOUNDTANK_IMP_OBJECT_CHANNEL_OPS
#define SOUNDTANK_IMP_OBJECT_CHANNEL_OPS


int create_imp_object_channel_cp(rtobject_t* rtobj);
int destroy_imp_object_channel_cp(rtobject_t* rtobj);

int init_instance_channel_cp(rtobject_t* rtobj, rtobject_instance_t* rtins);
int deinit_instance_channel_cp(rtobject_t* rtobj, rtobject_instance_t* rtins);



#endif

