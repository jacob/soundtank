/*
 * event map action callbacks coordination code
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


#ifndef EVENT_MAP_ACTION_CALLBACKS_COORDINATION_INCLUDE
#define EVENT_MAP_ACTION_CALLBACKS_COORDINATION_INCLUDE


void map_action_print_available_actions();

/*NOTE: map action init allocates a new action on the frame*/
int map_action_init(ev_route_frame_t* frame, int argc, char **argv);

char* map_action_get_func_name(map_action_t *action);
char* map_action_get_argv(map_action_t *action, int arg_index, rtobject_t *rtobj);


/*declare action init & callback functions here and list them in
  callback array that follows (soundtank_action_callbacks[])*/


extern int action_cb_jump(snd_seq_event_t *ev, ev_route_frame_t *frame);
extern int action_init_jump(ev_route_frame_t* frame, int argc, char **argv);
extern char* action_get_argv_jump(map_action_t* action, int arg_index, rtobject_t *rtobj);

extern int action_cb_set_control(snd_seq_event_t *ev, ev_route_frame_t *frame);
extern int action_init_set_control(ev_route_frame_t* frame, int argc, char **argv);
extern char* action_get_argv_set_control(map_action_t* action, int arg_index, rtobject_t *rtobj);

extern int action_cb_note_to_pitch(snd_seq_event_t *ev, ev_route_frame_t *frame);
extern int action_init_note_to_pitch(ev_route_frame_t* frame, int argc, char **argv);
extern char* action_get_argv_note_to_pitch(map_action_t* action, int arg_index, rtobject_t *rtobj);

extern int action_cb_set_note_flag(snd_seq_event_t *ev, ev_route_frame_t *frame);
extern int action_init_set_note_flag(ev_route_frame_t* frame, int argc, char **argv);
extern char* action_get_argv_set_note_flag(map_action_t* action, int arg_index, rtobject_t *rtobj);



/*NB: these structs were derived from the gnu readline example code*/

typedef struct {
  char* name;			/*function name*/
  map_action_callback_t func;	/*function address*/
  map_action_args_type_t args_type; /*function argument union member REMOVE?*/
  char* desc;			/*function description*/
  char* args_desc;
  map_action_init_t init;
  map_action_get_argv_t get_argv;
} action_callback_desc;




/*When included from soundtank.c, the primary reference define is set
  so that the callback array will be a local reference, when included
  from all other files it is not set and the array will be declared
  extern */

#ifdef SOUNDTANK_STRUCTS_PRIMARY_REF

action_callback_desc soundtank_action_callbacks[] = {
  { "jump", action_cb_jump, int_args,\
    "jump event to another test, map or rtobject",\
    "arg1: jump scope, is either 'test', 'map', 'rtobject', or 'child'. Child scope is only for signal paths. arg2: jump target, is an integer giving the list index for test & map scope, a pathname for rtobject scope, or an rtobject name (without any path) for child scope.",\
    action_init_jump, action_get_argv_jump},

  { "set", action_cb_set_control, int_args,\
    "set rtobject control value",\
    "arg1: control name or index. arg2: event parameter (channel, note, velocity, offvelocity, duration, ccparam, or ccvalue) or 'const' followed by a constant integer value. arg3: (optional) 'match', only effects instances that match the flag set using the flag action. arg4-6: (optional) 'shift' followed by constant float values min & max, shifts the value set from the range 0-127 to min-max.",\
    action_init_set_control, action_get_argv_set_control},

  { "pitch", action_cb_note_to_pitch, pitch_args,\
    "turn midi note into pitch using scale",\
    "arg1: control name or index. arg2: scale name or index. arg3: (optional) 'match', only effects instances that match the flag set using the flag action.",
    action_init_note_to_pitch, action_get_argv_note_to_pitch},

  { "flag", action_cb_set_note_flag, int_args,\
    "set an instance's midi note flag, allows use of 'match' in other actions to effect only one instance",\
    "arg1: event parameter to be matched (channel, note, velocity, offvelocity, duration, ccparam, or ccvalue). arg2: on or off, flag must be turned off when not needed any more.",\
    action_init_set_note_flag, action_get_argv_set_note_flag},


  { (char *)NULL, 0, int_args, (char *)NULL, (char *)NULL, 0, 0}
};

#else

extern action_callback_desc soundtank_action_callbacks[];

#endif

#endif
