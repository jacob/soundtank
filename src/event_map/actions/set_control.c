
#include <ctype.h>

#include <alsa/asoundlib.h>

#include "../../include.h"


char* action_get_argv_set_control(map_action_t* action, int arg_index, rtobject_t *rtobj){
  char* ret;
    
  if (arg_index == 0){
    /*arg 0: control name*/
    const control_t *control = rtobject_get_control(rtobj, action->args.set_args.control_index);

    if (!control) return 0;
    
    return strdup(control_get_desc_string(control));
      
  }else if (arg_index == 1){

    switch (action->args.set_args.ev_param){
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
    case EVENT_PARAM_NONE:
      return strdup("const");
    }

  }else if ((arg_index == 2)&&(action->args.set_args.ev_param == EVENT_PARAM_NONE)){

    if (!(ret = (char*)malloc(16*sizeof(char))))
      return 0;
    sprintf(ret, "%.3f", action->args.set_args.fixed_value);

    return ret;
  }

  else if ((action->args.set_args.match)&& \
	   ( ((arg_index == 2)&&(action->args.set_args.ev_param != EVENT_PARAM_NONE))||\
	     ((arg_index == 3)&&(action->args.set_args.ev_param == EVENT_PARAM_NONE)) )){
    /*the match argument is either 3rd or 4th depending on whether there's a fixed set value*/

    return strdup("match");

  }

  else{
    int shift_offset;

    shift_offset = 2;
    if (action->args.set_args.ev_param == EVENT_PARAM_NONE) ++shift_offset;
    if (action->args.set_args.match) ++shift_offset;
	    
    if (arg_index == shift_offset){

      /*first shift arg is shift*/
      if (!action->args.set_args.shift)
	return 0;
      return strdup("shift");

    }else if (arg_index == (shift_offset + 1)){

      /*second shift arg is range min*/
       if (!action->args.set_args.shift)
	return 0;

       if (!(ret = (char*)malloc(16*sizeof(char))))
	 return 0;
       sprintf(ret, "%.6f", action->args.set_args.shift_min);
       
       return ret;
       
    }else if (arg_index == (shift_offset + 2)){

      /*third shift arg is range max*/
       if (!action->args.set_args.shift)
	return 0;

       if (!(ret = (char*)malloc(16*sizeof(char))))
	 return 0;
       sprintf(ret, "%.6f", action->args.set_args.shift_max);
       
       return ret;

    }

  }

  return 0;
}

int action_init_set_control(ev_route_frame_t* frame, int argc, char **argv){
  int i;
  ll_head results;

  results = 0;

  if (argc < 3){
    printf("init args set control error: need more args (control# and parameter)\n");
    return -1;
  }

  /*find control to be set, match what the user gives to initial substrings*/
  if (rtobject_search_controls(frame->rtobj,argv[1], SEARCH_INITIAL_SUBSTRING, &results) <=0){
    printf("init action set control error: couldn't find control %s\n", argv[1]);
    return -1;
  }
 
  frame->action->args.set_args.control_index = *((int*)results->data);

  rtobject_free_search_results(&results);

  /*parse the event parameter name*/
  if (!(strcmp(argv[2],"channel"))){
    frame->action->args.set_args.ev_param = EVENT_PARAM_CHANNEL;
    
  }
  else if (!(strcmp(argv[2],"note"))){
    frame->action->args.set_args.ev_param = EVENT_PARAM_NOTE;
    
  }
  else if (!(strcmp(argv[2],"velocity"))){
    frame->action->args.set_args.ev_param = EVENT_PARAM_VELOCITY;
    
  }
  else if (!(strcmp(argv[2],"offvelocity"))){
    frame->action->args.set_args.ev_param = EVENT_PARAM_OFFVELOCITY;
    
  }
  else if (!(strcmp(argv[2],"duration"))){
    frame->action->args.set_args.ev_param = EVENT_PARAM_DURATION;
    
  }
  else if (!(strcmp(argv[2],"ccparam"))){
    frame->action->args.set_args.ev_param = EVENT_PARAM_CCPARAM;
    
  }
  else if (!(strcmp(argv[2],"ccvalue"))){
    frame->action->args.set_args.ev_param = EVENT_PARAM_CCVALUE;
    
  }
  else if (!(strcmp(argv[2],"const"))){
    /*handle setting control to a constant value independent of incoming event values*/

    if (argc < 4){
      printf("set control init error: not enough args, need a const value\n");
      return -1;
    }

    frame->action->args.set_args.ev_param = EVENT_PARAM_NONE;
    frame->action->args.set_args.fixed_value = atof(argv[3]);
    
  }
  else{
    printf("set control init error: unknown parameter %s\n", argv[2]);
    return -1;
  }

  /*handle "match" argument, which means only set value on instances
    whose note flag matches the incoming event's MIDI note #*/
  frame->action->args.set_args.match = 0x0; 

  for (i=0;i<argc;++i){
    if (!(strcmp(argv[i],"match"))){
      frame->action->args.set_args.match = 0x1; 
    }
  }

  /*handle shift argument, min & max*/
  frame->action->args.set_args.shift = 0x0; 

  for (i=0;i<argc;++i){
    if (!(strcmp(argv[i],"shift"))){
      frame->action->args.set_args.shift = 0x1; 
      break;
    }
  }
  
  if (frame->action->args.set_args.shift){

    if (argc < (i + 3)){
      printf("set control init error: not enough args for shift, need min & max\n");
      frame->action->args.set_args.shift = 0x0; 
      return -1;
    }

    frame->action->args.set_args.shift_min = atof(argv[i+1]);
    frame->action->args.set_args.shift_max = atof(argv[i+2]);
  
  }

  return 0;
}

int action_cb_set_control(snd_seq_event_t *ev, ev_route_frame_t *frame){
  float val;
  node_t* temp_node;

  /*extract desired parameter from event*/
  switch (frame->action->args.set_args.ev_param){

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
  case EVENT_PARAM_NONE:{
    val = frame->action->args.set_args.fixed_value;
    break;
  }
  default:
    return 0;
  }

  /*shift (and scale) value if action dictates*/
  if (frame->action->args.set_args.shift){
    
    /*val = min + percentage of 127 * length */
    val = frame->action->args.set_args.shift_min + ( (val/(float)127) * \
						     (frame->action->args.set_args.shift_max - frame->action->args.set_args.shift_min));

  }

  /*if no MIDI note matching is required, set control value in all instances*/
  if (!frame->action->args.set_args.match){

    for (temp_node=frame->rtobj->instance_list;temp_node;temp_node=temp_node->next){
      
      ((rtobject_instance_t*)temp_node->data)->control_list[frame->action->args.set_args.control_index] = val;
    
    }

  }else{
    /*for MIDI note matching, check note_flag field in all instances*/

    for (temp_node=frame->rtobj->instance_list;temp_node;temp_node=temp_node->next){
      
      if (((rtobject_instance_t*)temp_node->data)->note_flag == ev->data.note.note)
	((rtobject_instance_t*)temp_node->data)->control_list[frame->action->args.set_args.control_index] = val;
    
    }

  }

  return 0;
}
