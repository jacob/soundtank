
#include <stdio.h>
#include <alsa/asoundlib.h>

#include "../../include.h"




int test_cb_print_event_type(snd_seq_event_t* ev, ev_route_frame_t* frame){

  /*this function prints out a decription of the event type.*/

  /*these come from ALSA header: seq_event.h*/
  switch (ev->type){

  case SND_SEQ_EVENT_NOTE: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Note Event\n");
    break;
  }

  case SND_SEQ_EVENT_NOTEON: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Note On Event   Note# %d  Vel %d\n", ev->data.note.note, ev->data.note.velocity);
    break;
  }

  case SND_SEQ_EVENT_NOTEOFF: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Note Off Event  Note# %d  Vel %d\n",ev->data.note.note, ev->data.note.velocity);
    break;
  }

  case SND_SEQ_EVENT_KEYPRESS: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Keypress Event\n");
    break;
  }

  case SND_SEQ_EVENT_CONTROLLER: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("CC Event  CC%d Val%d\n", ev->data.control.param, ev->data.control.value);
    break;
  }

  case SND_SEQ_EVENT_PGMCHANGE: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Program Change Event\n");
    break;
  }

  case SND_SEQ_EVENT_CHANPRESS: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Channel Pressure Event\n");
    break;
  }

  case SND_SEQ_EVENT_PITCHBEND: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Pitch Bend Event\n");
    break;
  }

  case SND_SEQ_EVENT_NONREGPARAM: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Non-Registered Parameter Event\n");
    break;
  }

  case SND_SEQ_EVENT_REGPARAM: {
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Registered Parameter Event\n");
    break;
  }

  case SND_SEQ_EVENT_SENSING: {
    /*drop those pesky sensing events*/
    break;
  }

  default:{
    /*first print name of rtobject that received it*/
    printf("%s: ", rtobject_get_name(frame->rtobj));
    printf("Unknown Event Type\n");
    break;
  }

  }
  
  return 0;
}
