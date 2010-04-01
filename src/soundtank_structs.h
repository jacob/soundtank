/*
 * soundtank - application-wide data structures
 *
 * Copyright Jacob Robbins 2004
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


/*NOTE: you should _only_ include this header if you really really
  need to use one of the app-wide structs. That's why it's not in
  include.h*/


/*When included from soundtank.c, the primary reference define is set
  so that these will be local references, when included from all other
  files it is not set and they will be declared extern */

#ifndef SOUNDTANK_STRUCTS_PRIMARY_REF

#define SOUNDTANK_STRUCTS_PRIMARY_REF extern

#else

#undef SOUNDTANK_STRUCTS_PRIMARY_REF 
#define SOUNDTANK_STRUCTS_PRIMARY_REF

#endif




/***  Soundtank's Application-Wide Structures */

/************/
/*Assorted Things*/
/************/



SOUNDTANK_STRUCTS_PRIMARY_REF int rt_error_readout; 
/*controls screen display of realtime errors*/

SOUNDTANK_STRUCTS_PRIMARY_REF int msgid; 
/*holds id for using system V ipc messages*/

SOUNDTANK_STRUCTS_PRIMARY_REF void** hot_swap_pointer_new;
SOUNDTANK_STRUCTS_PRIMARY_REF void** hot_swap_pointer_old;
/*used to have engine make changes to structures that it uses*/

SOUNDTANK_STRUCTS_PRIMARY_REF ll_head scale_list;
/*holds scales used to turn MIDI notes into pitches*/

/************/
/*Debugging*/
/************/

SOUNDTANK_STRUCTS_PRIMARY_REF int debug_readout; 
/*controls very verbose stepbystep display*/

SOUNDTANK_STRUCTS_PRIMARY_REF int debug_process_function; 
/*controls engine debug display*/


/************/
/*Engine*/
/************/

SOUNDTANK_STRUCTS_PRIMARY_REF engine_t* soundtank_engine;


/************/
/*Realtime Processing List*/
/************/
SOUNDTANK_STRUCTS_PRIMARY_REF int process_list_size;
SOUNDTANK_STRUCTS_PRIMARY_REF rtobject_instance_t** process_list;
SOUNDTANK_STRUCTS_PRIMARY_REF int swap_process_list_size; 
/*used for modifying process list*/
SOUNDTANK_STRUCTS_PRIMARY_REF rtobject_instance_t** swap_process_list;  
/*used for modifying process list*/


/************/
/*Master Signal Path*/
/************/
SOUNDTANK_STRUCTS_PRIMARY_REF signal_path_t* master_path;
SOUNDTANK_STRUCTS_PRIMARY_REF rtobject_t* master_path_rtobject;
SOUNDTANK_STRUCTS_PRIMARY_REF signal_path_t* curr_path;


/************/
/*Addressing List*/
/************/
SOUNDTANK_STRUCTS_PRIMARY_REF int rtobject_address_list_size;
SOUNDTANK_STRUCTS_PRIMARY_REF rtobject_t** rtobject_address_list;
SOUNDTANK_STRUCTS_PRIMARY_REF int swap_rtobject_address_list_size;      
/*used for changing size of address list*/
SOUNDTANK_STRUCTS_PRIMARY_REF rtobject_t** swap_rtobject_address_list;  
/*used for changing size of address list*/


/************/
/*Buffer List*/
/************/
SOUNDTANK_STRUCTS_PRIMARY_REF channel_t* null_read;
SOUNDTANK_STRUCTS_PRIMARY_REF channel_t* null_write;
SOUNDTANK_STRUCTS_PRIMARY_REF ll_head buffer_list;


/************/
/*ALSA Sequencer Client*/
/************/
SOUNDTANK_STRUCTS_PRIMARY_REF snd_seq_t* alsa_seq_client_handle;


/************/
/*Current Command Line*/
/************/
SOUNDTANK_STRUCTS_PRIMARY_REF char dyn_command[128];



/***  End of Soundtank's Application-Wide Structures ***/


#undef SOUNDTANK_STRUCTS_PRIMARY_REF
