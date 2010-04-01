/*
 * event map test callbacks coordination code
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


#ifndef EVENT_MAP_TEST_CALLBACKS_COORDINATION_INCLUDE
#define EVENT_MAP_TEST_CALLBACKS_COORDINATION_INCLUDE

void map_test_print_available_tests();

int map_test_init(ev_route_frame_t* frame, int argc, char **argv);

char* map_test_get_func_name(const map_test_t *test);
char* map_test_get_argv(const map_test_t *test, int arg_index, rtobject_t *rtobj);



/*declare test init & callback functions here and list them in callback array
  that follows (soundtank_test_callbacks[])*/
extern int test_cb_print_event_type(snd_seq_event_t* ev, ev_route_frame_t* frame);

extern int test_cb_true(snd_seq_event_t* ev, ev_route_frame_t* frame);

extern int test_cb_event_type(snd_seq_event_t* ev, ev_route_frame_t* frame);
extern int test_init_event_type(ev_route_frame_t* frame, int argc, char **argv);
extern char* test_get_argv_event_type(const map_test_t* test, int arg_index, rtobject_t *rtobj);


extern int test_cb_event_param_in_range(snd_seq_event_t* ev, ev_route_frame_t* frame);
extern int test_init_event_param_in_range(ev_route_frame_t* frame, int argc, char **argv);
extern char* test_get_argv_event_param_in_range(const map_test_t* test, int arg_index, rtobject_t *rtobj);



/*NB: these structs were derived from the gnu readline example code*/

typedef struct {
  char* name;			/*function name*/
  map_test_callback_t func;	/*function address*/
  char* desc;			/*function description*/
  char* args_desc;
  map_test_init_t init;
  map_test_get_argv_t get_argv;
} test_callback_desc;




/*When included from soundtank.c, the primary reference define is set
  so that the callback array will be a local reference, when included
  from all other files it is not set and the array will be declared
  extern */

#ifdef SOUNDTANK_STRUCTS_PRIMARY_REF

test_callback_desc soundtank_test_callbacks[] = {
  { "print", test_cb_print_event_type,\
    "print MIDI event info to screen",\
    "no args",
    0, 0},

  { "true", test_cb_true,\
    "always returns true",\
    "no args",
    0, 0},

  { "type", test_cb_event_type, \
    "test MIDI event type",\
    "arg1: note, noteon, noteoff, keypress, cc, program, chanpressure, pitchbend, nonregparam, or regparam. arg2: (optional) cc#, for type cc only",\
    test_init_event_type, test_get_argv_event_type},

  { "range", test_cb_event_param_in_range, \
    "test MIDI event parameter in range", \
    "arg1: channel, note, velocity, offvelocity, duration, ccparam, or ccvalue. arg2: min. arg3: max.",\
    test_init_event_param_in_range, test_get_argv_event_param_in_range},

  { (char *)NULL, 0, (char *)NULL, (char *)NULL, 0, 0}
};

#else

extern test_callback_desc soundtank_test_callbacks[];

#endif

#endif
