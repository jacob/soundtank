
#include <ctype.h>

#include <alsa/asoundlib.h>

#include "../../include.h"


char* action_get_argv_note_to_pitch(map_action_t* action, int arg_index, rtobject_t *rtobj){
  if (arg_index > 2) return 0;

  if (arg_index == 0){
    /*arg 0: control name*/
    const control_t *control = rtobject_get_control(rtobj, action->args.pitch_args.control_index);

    if (!control) return 0;
    
    return strdup(control_get_desc_string(control));

  }else if (arg_index == 1){
    char* ret;

    if (!action->args.pitch_args.scale)
      return 0;

    if (!(ret = strdup(scale_get_name(action->args.pitch_args.scale))))
      return 0;
    
    return ret;
  }else if (arg_index == 2){

    if (!action->args.pitch_args.match)
      return 0;
    return strdup("match");

  }

  return 0;
}

int action_init_note_to_pitch(ev_route_frame_t* frame, int argc, char **argv){
  ll_head results;

  results = 0;

  if (argc < 3){
    printf("init args pitch error: need more args (control# and parameter)\n");
    return -1;
  }

  /*find control to be set, match what the user gives to initial substrings*/
  if (rtobject_search_controls(frame->rtobj,argv[1], SEARCH_INITIAL_SUBSTRING, &results) <=0){
    printf("init action set control error: couldn't find control %s\n", argv[1]);
    return -1;
  }
 
  frame->action->args.pitch_args.control_index = *((int*)results->data);

  rtobject_free_search_results(&results);

  /*parse the scale name*/
  if (string_is_number(argv[2])){

    if (!(frame->action->args.pitch_args.scale = get_scale(atoi(argv[2])))){
      printf("init args pitch error: couldn't find scale %s\n", argv[2]);
      return -1;
    }

  }else{

    if (!(frame->action->args.pitch_args.scale = get_scale_by_name(argv[2]))){
      printf("init args pitch error: couldn't find scale %s\n", argv[2]);
      return -1;
    }

  }

  if ((argc > 3)&&(!(strcmp(argv[3],"match")))){
    
    frame->action->args.pitch_args.match = 0x1;

  }else{

    frame->action->args.pitch_args.match = 0x0;

  }

  return 0;
}


int action_cb_note_to_pitch(snd_seq_event_t *ev, ev_route_frame_t *frame){
  float val;
  node_t* temp_node;

  if (!frame->action->args.pitch_args.scale)
    return 0;

  val = rt_note_to_pitch(frame->action->args.pitch_args.scale,\
			   ev->data.note.note);
  
  /*if no MIDI note matching is required, set control value in all instances*/
  if (!frame->action->args.pitch_args.match){

    for (temp_node=frame->rtobj->instance_list;temp_node;temp_node=temp_node->next){
    
      ((rtobject_instance_t*)temp_node->data)->control_list[frame->action->args.int_args.arg1] = val;
    
    }

  }else{

    /*for MIDI note matching, check note_flag field in all instances*/
    for (temp_node=frame->rtobj->instance_list;temp_node;temp_node=temp_node->next){
    
      if (((rtobject_instance_t*)temp_node->data)->note_flag == ev->data.note.note)
	((rtobject_instance_t*)temp_node->data)->control_list[frame->action->args.int_args.arg1] = val;
    
    }


  }

  return 0;
}
