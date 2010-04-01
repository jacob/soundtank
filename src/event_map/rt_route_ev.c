/*
 * event routing code
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

#include "../include.h"


void rt_event_map_route_event(snd_seq_event_t* ev){

  /*the frame stores the event map section that's being evaluated*/
  ev_route_frame_t frame;
  node_t* curr_node;

  /*TODO: work on copy of event*/

  /*prime pump with initial targets (rtobj,map,test) ... */

  /*get the initial object*/
  if (!(frame.rtobj = get_rtobject_from_address( ev->dest.port )))
      return;

  /*get the initial map*/
  if (rtobject_get_map_list_size(frame.rtobj)){
    if ((rtobject_get_map(frame.rtobj, &frame.map, 0)) < 0)
      return;
  }else{
    return;
  }
  
  /*get the initial test (raw access used to include terminating null entry)*/
  frame.test = (map_test_t*)generic_array_get_data_pointer(frame.map->test_array);

  /*actions which adjust the frame will jump here so their changes
    take effect. They must set the frame flag to jump here.*/
 loop_entry_label:

  /*frame adjustments can lead to their being no map, which ends routing*/
  if (!frame.map) return;

  if (frame.flags) frame.flags = 0;

  /*note that the test array is terminated by an empty entry. this can
    be checked by looking at the callback fxn field.*/
  for ( ; (frame.test->fxn) ; ++frame.test){

    /*run the test callback*/
    if ( (frame.test->fxn(ev, &frame)) > 0){

      /*go through all the actions attached to the successful test*/
      for (curr_node=frame.test->action_list; \
	     curr_node; \
	     curr_node=curr_node->next){
	
	frame.action = (map_action_t*)curr_node->data;
	
	/*run the action callback*/
	frame.action->fxn(ev, &frame);

	/*check to see if action invalidated the frame, if so re-enter
          loop from top so frame changes take effect*/
	if (frame.flags) goto loop_entry_label;

      }

      return;

    }

  }


  /*done*/
}


