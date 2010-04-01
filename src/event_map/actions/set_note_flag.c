
#include <stdio.h>
#include <ctype.h>

#include <alsa/asoundlib.h>

#include "../../include.h"


char* action_get_argv_set_note_flag(map_action_t* action, int arg_index, rtobject_t *rtobj){

  if (arg_index > 1) return 0;

  if (arg_index == 0){
 
    switch (action->args.int_args.arg1){
    case EVENT_PARAM_CHANNEL:
      return strdup("channel");
    case EVENT_PARAM_NOTE:
      return strdup("note");
    case EVENT_PARAM_VELOCITY:
      return strdup("velocity");
    case EVENT_PARAM_OFFVELOCITY:
      return strdup("offvelocity");
    case EVENT_PARAM_DURATION:
      return strdup("duration");
    case EVENT_PARAM_CCPARAM:
      return strdup("ccparam");
    case EVENT_PARAM_CCVALUE:
      return strdup("ccvalue");
    default:
      return 0;
    }

  }else if (arg_index == 1){

    if (action->args.int_args.arg2 > 0)
      return strdup("on");
    else
      return strdup("off");

  }

  return 0;
}

int action_init_set_note_flag(ev_route_frame_t* frame, int argc, char **argv){

  if (argc < 3){
    printf("init args set note flag error: need more args (parameter and on|off)\n");
    return -1;
  }
 
  /*parse the event parameter name*/
  if (!(strcmp(argv[1],"channel"))){
    frame->action->args.int_args.arg1 = EVENT_PARAM_CHANNEL;
    
  }
  else if (!(strcmp(argv[1],"note"))){
    frame->action->args.int_args.arg1 = EVENT_PARAM_NOTE;
    
  }
  else if (!(strcmp(argv[1],"velocity"))){
    frame->action->args.int_args.arg1 = EVENT_PARAM_VELOCITY;
    
  }
  else if (!(strcmp(argv[1],"offvelocity"))){
    frame->action->args.int_args.arg1 = EVENT_PARAM_OFFVELOCITY;
    
  }
  else if (!(strcmp(argv[1],"duration"))){
    frame->action->args.int_args.arg1 = EVENT_PARAM_DURATION;
    
  }
  else if (!(strcmp(argv[1],"ccparam"))){
    frame->action->args.int_args.arg1 = EVENT_PARAM_CCPARAM;
    
  }
  else if (!(strcmp(argv[1],"ccvalue"))){
    frame->action->args.int_args.arg1 = EVENT_PARAM_CCVALUE;
    
  }
  else{
    printf("set note flag init error: unknown parameter %s\n", argv[1]);
    return -1;
  }

  if (!(strcmp(argv[2],"on"))){
     frame->action->args.int_args.arg2 = 1;
  }else if (!(strcmp(argv[2],"off"))){
     frame->action->args.int_args.arg2 = 0;
  }else{
    printf("set note flag init error: unknown argument %s\n", argv[2]);
    return -1;
  }
   
  return 0;
}

int action_cb_set_note_flag(snd_seq_event_t *ev, ev_route_frame_t *frame){
  unsigned char val;
  node_t* temp_node;

  /*extract desired parameter from event*/
  switch (frame->action->args.int_args.arg1){

  case EVENT_PARAM_CHANNEL:{
    /*this works for notes & control events because channel is the
      first member of both structures*/
    val = ev->data.note.channel;
    break;
  }
  case EVENT_PARAM_NOTE:{
    val = ev->data.note.note;
    break;
  }
  case EVENT_PARAM_VELOCITY:{
    val = ev->data.note.velocity;
    break;
  }
  case EVENT_PARAM_OFFVELOCITY:{
    val = ev->data.note.off_velocity;
    break;
  }
  case EVENT_PARAM_DURATION:{
    val = ev->data.note.duration;
    break;
  }
  case EVENT_PARAM_CCPARAM:{
    val = ev->data.control.param;
    break;
  }
  case EVENT_PARAM_CCVALUE:{
    val = ev->data.control.value;
    break;
  }
  default:
    return 0;
  }

  /*check if we are setting or unsetting flag*/
  if (frame->action->args.int_args.arg2 > 0){

    /*set MIDI note flag in first inactive instance*/
    for (temp_node=frame->rtobj->instance_list;temp_node;temp_node=temp_node->next){
    
      if (((rtobject_instance_t*)temp_node->data)->control_list[0] <= 0){
	((rtobject_instance_t*)temp_node->data)->note_flag = val;
	break;
      }
      
      /*if there aren't any muted instances, commandeer the last instance*/
      if (!temp_node->next)
	((rtobject_instance_t*)temp_node->data)->note_flag = val;

    }

  }else{

    /*unset flag wherever it is set*/
    for (temp_node=frame->rtobj->instance_list;temp_node;temp_node=temp_node->next){

      if (((rtobject_instance_t*)temp_node->data)->note_flag == val)
	((rtobject_instance_t*)temp_node->data)->note_flag = 0;

    }

  }

  return 0;
}
