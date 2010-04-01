/*
 * imp object JACK extern in/out code
 *
 * Copyright 2004 Jacob Robbins
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


#ifndef IMP_OBJECT_JACK_EXTERN_INCLUDE
#define IMP_OBJECT_JACK_EXTERN_INCLUDE

#include <jack/jack.h>

typedef struct jack_extern jack_extern_t;
struct jack_extern{

  int owner_object_address;
  jack_port_t* jack_port;

};


jack_extern_t* imp_object_jack_extern_alloca();
void imp_object_jack_extern_dealloca(jack_extern_t* oldobj);

int create_imp_object_jack_extern_out(rtobject_t* rtobj);
int destroy_imp_object_jack_extern_out(rtobject_t* rtobj);

int create_imp_object_jack_extern_in(rtobject_t* rtobj);
int destroy_imp_object_jack_extern_in(rtobject_t* rtobj);

int init_instance_jack_extern_out(rtobject_t* rtobj, rtobject_instance_t* rtins);
int deinit_instance_jack_extern_out(rtobject_t* rtobj, rtobject_instance_t* rtins);

int init_instance_jack_extern_in(rtobject_t* rtobj, rtobject_instance_t* rtins);
int deinit_instance_jack_extern_in(rtobject_t* rtobj, rtobject_instance_t* rtins);



#endif
