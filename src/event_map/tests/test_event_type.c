
#include <string.h>
#include <stdio.h>
#include <alsa/asoundlib.h>

#include "../../include.h"


char* test_get_argv_event_type(const map_test_t* test, int arg_index, rtobject_t *rtobj){

  if (arg_index > 1) return 0;

  if (arg_index == 0){

    switch (test->arg1){
    case SND_SEQ_EVENT_NOTE:
      return strdup("note");
    case SND_SEQ_EVENT_NOTEON:
      return strdup("noteon");
    case SND_SEQ_EVENT_NOTEOFF:
      return strdup("noteoff");
    case SND_SEQ_EVENT_KEYPRESS:
      return strdup("keypress");
    case SND_SEQ_EVENT_CONTROLLER:
      return strdup("cc");
    case SND_SEQ_EVENT_PGMCHANGE:
      return strdup("program");
    case SND_SEQ_EVENT_CHANPRESS:
      return strdup("chanpressure");
    case SND_SEQ_EVENT_PITCHBEND:
      return strdup("pitchbend");
    case SND_SEQ_EVENT_NONREGPARAM:
      return strdup("nonregparam");
    case SND_SEQ_EVENT_REGPARAM:
      return strdup("regparam");
    }

  }else if (arg_index == 1){
    char* ret;

    if (test->arg1 != SND_SEQ_EVENT_CONTROLLER)
      return 0;
    
    if (!(ret = (char*)malloc(6*sizeof(char))))
      return 0;
    sprintf(ret, "%d", test->arg2);
    
    return ret;
  }
  
  return 0;
}

int test_init_event_type(ev_route_frame_t* frame, int argc, char **argv){

  if (argc < 2){
    printf("init args event type error: need more args\n");
    return -1;
  }

  if (!(strcmp(argv[1],"note"))){
    frame->test->arg1 = SND_SEQ_EVENT_NOTE;
    
  }
  else if (!(strcmp(argv[1],"noteon"))){
    frame->test->arg1 = SND_SEQ_EVENT_NOTEON;
    
  }
  else if (!(strcmp(argv[1],"noteoff"))){
    frame->test->arg1 = SND_SEQ_EVENT_NOTEOFF;
    
  }
  else if (!(strcmp(argv[1],"keypress"))){
    frame->test->arg1 = SND_SEQ_EVENT_KEYPRESS;
    
  }
  else if (!(strcmp(argv[1],"cc"))){
    frame->test->arg1 = SND_SEQ_EVENT_CONTROLLER;

    if (argc > 2){

      frame->test->arg2 = atoi(argv[2]);

    }else{
      
      frame->test->arg2 = -1; /*this means check for any cc events*/

    }
    
  }
  else if (!(strcmp(argv[1],"program"))){
    frame->test->arg1 = SND_SEQ_EVENT_PGMCHANGE;
    
  }
  else if (!(strcmp(argv[1],"chanpressure"))){
    frame->test->arg1 = SND_SEQ_EVENT_CHANPRESS;
    
  }
  else if (!(strcmp(argv[1],"pitchbend"))){
    frame->test->arg1 = SND_SEQ_EVENT_PITCHBEND;
    
  }
  else if (!(strcmp(argv[1],"nonregparam"))){
    frame->test->arg1 = SND_SEQ_EVENT_NONREGPARAM;
    
  }
  else if (!(strcmp(argv[1],"regparam"))){
    frame->test->arg1 = SND_SEQ_EVENT_REGPARAM;
    
  }

  else{
    printf("init args event type err: unknown parameter name, %s\n",argv[1]);
    return -1;
  }


  return 0;
}


int test_cb_event_type(snd_seq_event_t* ev, ev_route_frame_t* frame){
  /*this function checks if an event is of a certain type*/

  if (ev->type == frame->test->arg1){

    if (ev->type == SND_SEQ_EVENT_CONTROLLER){

      /*test arg2 val < 0 means just check for any cc events*/
      if (frame->test->arg2 < 0) return 1;

      /*test arg2 val >= 0 means match the cc #*/
      if (frame->test->arg2 == ev->data.control.param) return 1;


    }else{
      
      return 1;

    }
  
  }

  return 0;
}
