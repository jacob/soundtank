
#include <string.h>
#include <stdio.h>
#include <alsa/asoundlib.h>


#include "../../include.h"


char* test_get_argv_event_param_in_range(const map_test_t* test, int arg_index, rtobject_t *rtobj){
  
  if (arg_index > 2) return 0;

  if (arg_index == 0){
    switch (test->arg1){
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
    }

  }else if (arg_index == 1){
    char* ret;

    if (!(ret = (char*)malloc(6*sizeof(char))))
      return 0;
    sprintf(ret, "%d", test->arg2);

    return ret;
  }else if (arg_index == 2){
    char* ret;

    if (!(ret = (char*)malloc(6*sizeof(char))))
      return 0;
    sprintf(ret, "%d", test->arg3);

    return ret;
  }

  return 0;
}

int test_init_event_param_in_range(ev_route_frame_t* frame, int argc, char **argv){

  if (argc < 4){
    printf("init args event param in range error: need more args\n");
    return -1;
  }

  if (!(strcmp(argv[1],"channel"))){
    frame->test->arg1 = EVENT_PARAM_CHANNEL;
    
  }
  else if (!(strcmp(argv[1],"note"))){
    frame->test->arg1 = EVENT_PARAM_NOTE;
    
  }
  else if (!(strcmp(argv[1],"velocity"))){
    frame->test->arg1 = EVENT_PARAM_VELOCITY;
    
  }
  else if (!(strcmp(argv[1],"offvelocity"))){
    frame->test->arg1 = EVENT_PARAM_OFFVELOCITY;
    
  }
  else if (!(strcmp(argv[1],"duration"))){
    frame->test->arg1 = EVENT_PARAM_DURATION;
    
  }
  else if (!(strcmp(argv[1],"ccparam"))){
    frame->test->arg1 = EVENT_PARAM_CCPARAM;
    
  }
  else if (!(strcmp(argv[1],"ccvalue"))){
    frame->test->arg1 = EVENT_PARAM_CCVALUE;
    
  }

  else{
    printf("param in range init error: unknown parameter %s\n", argv[1]);
    return -1;
  }

  /*set range min & max*/
  frame->test->arg2 = atoi(argv[2]);
  frame->test->arg3 = atoi(argv[3]);

  return 0;
}

int test_cb_event_param_in_range(snd_seq_event_t* ev, ev_route_frame_t* frame){

  switch (frame->test->arg1){

  case EVENT_PARAM_CHANNEL:{

    /*this works for notes & control events because channel is the
      first member of both structures*/
    if ( (ev->data.note.channel >= frame->test->arg2)&&\
	 (ev->data.note.channel <= frame->test->arg3) )
      return 1;

    break;
  }
  case EVENT_PARAM_NOTE:{

    if ( (ev->data.note.note >= frame->test->arg2)&&\
	 (ev->data.note.note <= frame->test->arg3) ){
      return 1;
    }

    break;
  }
  case EVENT_PARAM_VELOCITY:{
 
    if ( (ev->data.note.velocity >= frame->test->arg2)&&\
	 (ev->data.note.velocity <= frame->test->arg3) )
      return 1;

    break;
  }
  case EVENT_PARAM_OFFVELOCITY:{

    if ( (ev->data.note.off_velocity >= frame->test->arg2)&&\
	 (ev->data.note.off_velocity <= frame->test->arg3) )
      return 1;

    break;
  }
  case EVENT_PARAM_DURATION:{

    if ( (ev->data.note.duration >= frame->test->arg2)&&\
	 (ev->data.note.duration <= frame->test->arg3) )
      return 1;

    break;
  }
  case EVENT_PARAM_CCPARAM:{

    if ( (ev->data.control.param >= frame->test->arg2)&&\
	 (ev->data.control.param <= frame->test->arg3) )
      return 1;

    break;
  }
  case EVENT_PARAM_CCVALUE:{

    if ( (ev->data.control.value >= frame->test->arg2)&&\
	 (ev->data.control.value <= frame->test->arg3) )
      return 1;

    break;
  }
  default:
    return 0;
  }

  return 0;
}
