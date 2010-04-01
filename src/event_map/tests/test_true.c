

#include <alsa/asoundlib.h>

#include "../../include.h"



int test_cb_true(snd_seq_event_t* ev, ev_route_frame_t* frame){
  return 1;
}
