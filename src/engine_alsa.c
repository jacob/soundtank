/*
 * Soundtank ALSA (Advanced Linux Sound Architecture) engine code
 *
 * Copyright 2003-2004 Jacob Robbins
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



#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>

#include "include.h"
#include "soundtank_structs.h"

static pthread_t engine_thread;



static snd_pcm_t *playback_handle;


/*
 sigset_t set;


   Block signals to this thread
  sigfillset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTSTP);
  sigaddset(&set, SIGCONT);
  if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0)
    DEBUG("pthread_sigmask failed");


*/

int engine_alsa_initialize(engine_t* engine){
  snd_pcm_hw_params_t* hw_params;
  snd_pcm_sw_params_t* sw_params;
  int err,updn;

  /*validate engine parameters before using ALSA api*/
  if (!engine->device_name){

    printf("no alsa device selected, using default 'plughw:0,0'\n");
    if (!(engine->device_name = strdup("plughw:0,0"))) {
      printf("memory error allocating string\n"); 
      return -1;
    }

  }

  if (!engine->sample_rate){
    
    printf("no sample rate specified, using default 44,100 hz\n");
    engine->sample_rate = 44100;
    
  }

  if (!engine->num_channels){
    
    printf("no num channels specified, using default 2\n");
    engine->num_channels = 2;
    
  }

  if (!engine->num_periods){
    
    printf("no num periods specified, using default 2\n");
    engine->num_periods = 2;
    
  }
  
  if (!engine->period_size){
    
    printf("no period size specified, using default 1024\n");
    engine->period_size = 1024;
    
  }
  
  if (!engine->interleaved){
    
    engine->interleaved = SND_PCM_ACCESS_RW_NONINTERLEAVED;
    
  }else{
    
    engine->interleaved = SND_PCM_ACCESS_RW_INTERLEAVED;
    
  }
  
  /* avail min defaults to period size*/

  /*the remainder of this function is adapted from Paul Davis' example
    code at www.alsa-project.org*/

  if ((err = snd_pcm_open (&playback_handle, engine->device_name,\
			   SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK)) < 0) {
    fprintf (stderr, "cannot open audio device (%s)\n", snd_strerror (err));
    exit (1);
		}
  /*plughw:0,0*/
  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  
  if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  
  /*SND_PCM_ACCESS_RW_NONINTERLEAVED, SND_PCM_ACCESS_RW_INTERLEAVED*/
  if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params,\
					   engine->interleaved)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  
  if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params,\
					   SND_PCM_FORMAT_S16_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
	     snd_strerror (err));
    exit (1);
  }else{
    engine->sample_size=sizeof(short int);
  }
  
  if ((err = snd_pcm_hw_params_set_rate_near (playback_handle,\
					      hw_params,\
					      &(engine->sample_rate),\
					      0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  
  if ((err = snd_pcm_hw_params_set_channels (playback_handle,\
					     hw_params,\
					     engine->num_channels)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
	     snd_strerror (err));
    exit (1);
  }
  
  if (engine->buffer_size){
    if ((err = snd_pcm_hw_params_set_buffer_size_near (playback_handle,\
						       hw_params,\
						       &(engine->buffer_size))) < 0) {
      fprintf (stderr, "cannot set buffer size (%s)\n",
	       snd_strerror (err));
      exit (1);
    }
  }

  if (engine->num_periods){
    if ((err = snd_pcm_hw_params_set_periods(playback_handle,\
					     hw_params,\
					     engine->num_periods,\
					     0 )) < 0) {
      fprintf (stderr, "cannot set num periods (%s)\n",
	       snd_strerror (err));
      exit (1);
    }
  }
  
  if (engine->period_size){
    if ((err = snd_pcm_hw_params_set_period_size_near (playback_handle,\
						       hw_params,\
						       &(engine->period_size),\
						       0 )) < 0) {
      fprintf (stderr, "cannot set period size (%s)\n",
	       snd_strerror (err));
      exit (1);
    }
  }
  
  /*try to set all the soundcard values we just went through*/
  if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  /*check what soundcard accepted*/
  if ((err = snd_pcm_hw_params_get_period_size(hw_params, &(engine->period_size), &updn)) < 0){
    printf("Unable to get period size for verification: %s\n", snd_strerror(err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_get_periods(hw_params, &(engine->num_periods), &updn)) < 0){
    printf("Unable to get period count for verification: %s\n", snd_strerror(err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_get_buffer_size(hw_params, &(engine->buffer_size) )) < 0){
    printf("Unable to get buffer size for verification: %s\n", snd_strerror(err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_get_rate(hw_params, &(engine->sample_rate), &updn)) < 0){
    printf("Unable to get sample rate for verification: %s\n", snd_strerror(err));
    exit (1);
  }   

  if ((err = snd_pcm_hw_params_get_channels(hw_params, &(engine->num_channels))) < 0){   
    printf("Unable to get channel count for verification: %s\n", snd_strerror(err));
    exit (1);
  }   


  snd_pcm_hw_params_free (hw_params);


  /*set soundcard driver values*/
  if ((err = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
    fprintf (stderr, "cannot allocate software parameters structure (%s)\n", snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_sw_params_current(playback_handle,sw_params)) < 0) {
    fprintf (stderr, "cannot initialize software parameters structure (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  /*available min defaults to period size, should it _ever_ be different?*/
  if (!engine->avail_min) {
    engine->avail_min = engine->period_size;
  }
  if ((err = snd_pcm_sw_params_set_avail_min(playback_handle,sw_params, engine->period_size)) < 0) {
    fprintf (stderr, "cannot set minimum available count (%s)\n", snd_strerror (err));
    exit (1);
  }

  /*0U*/
  if ((err = snd_pcm_sw_params_set_start_threshold(playback_handle,sw_params,0U)) < 0) {
    fprintf (stderr, "cannot set start mode (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  /* align all transfers to 1 sample */
  if ((err = snd_pcm_sw_params_set_xfer_align(playback_handle, sw_params, 1)) < 0) {
    printf("Unable to set transfer align for playback: %s\n", snd_strerror(err));
    return err;
  }

  /*try to set software paramaters we just went through*/
  if ((err = snd_pcm_sw_params(playback_handle,sw_params)) < 0) {
    fprintf (stderr, "cannot set software parameters (%s)\n",
	     snd_strerror (err));
    exit (1);
  }

  /*check what soundcard accepted*/
  if ((err = snd_pcm_sw_params_get_avail_min(sw_params,&engine->avail_min)) < 0) {
    fprintf (stderr, "cannot get minimum available count (%s)\n", snd_strerror (err));
    exit (1);
  }

 
  snd_pcm_sw_params_free(sw_params);

  if (engine_alsa_initialize_buffer(engine) < 0){
    printf("engine alsa init error: couldn't initialize buffer\n");
    return -1;
  }

  return 0; 
}

int engine_alsa_initialize_buffer(engine_t* engine){
  int i;
  signed short *ptr;
  pair_i_v_t buf_entry;


  /*allocate buffer for writing to device, must be in external
    interface's format (ie JACK or ALSA device)*/
  /*currently only 16 bit format is supported TODO: add 24 bit format*/
  if (!(engine->buffer = malloc(engine->sample_size * engine->period_size * engine->num_channels))){
    fprintf (stderr, "ERROR: failed to allocate hardware buffer array\n");
    return -1;
  }
  
  /*zero out buffer for writing to device*/
  ptr = (signed short *) engine->buffer;
  for (i=0;i<(engine->period_size * engine->num_channels);++i)
    ptr[i] = 0;
    
  /*create and fill write buffer array (references used to track
    device's extern_outs) */
  if (generic_array_set_size(engine->write_buffer_array, engine->num_channels) < 0) {
    fprintf (stderr, "cannot resize hardware buffer array\n");
    return -1;
  }
 
  for (i=0;i<engine->num_channels;++i){

    if (generic_array_get_element(engine->write_buffer_array, i, &buf_entry) < 0){
	fprintf (stderr, "cannot access hardware buffer array element %d\n",i);
	return -1;
    }

    /*make sure buffer entry is marked as free so an rtobject can
      attach to it*/
    buf_entry.owner_object_address = -1;

    if (engine->interleaved == SND_PCM_ACCESS_RW_INTERLEAVED){
      /*interleaved mode*/
      buf_entry.data = engine->buffer + (engine->sample_size *  i);
    }else{
      /*noninterleaved mode*/
      buf_entry.data = engine->buffer + (engine->sample_size * engine->period_size * i);
    }

    if (generic_array_set_element(engine->write_buffer_array, i, &buf_entry) < 0){
	fprintf (stderr, "cannot set hardware buffer array element %d\n",i);
	return -1;
    }
       
  }

  /*engine is ready to go, set state variable to indicate so*/
  engine->state = ENGINE_STATE_INACTIVE;

  return 0;
}

void* engine_alsa_attach_to_device_output_channel(engine_t* engine, int channel, int object_address){
  pair_i_v_t buf_entry;

  if (!engine->write_buffer_array) return 0;

  if (generic_array_get_element(engine->write_buffer_array, channel, &buf_entry) < 0){
    fprintf(stderr,"error accessing engine write buffer array: element out of range\n");
    return 0;
  }

  if (buf_entry.owner_object_address != -1){
    fprintf(stderr,"error: that engine write buffer is already owned\n");
    return 0;
  }else{
    buf_entry.owner_object_address = object_address;
  }

  if (generic_array_set_element(engine->write_buffer_array, channel, &buf_entry) < 0){
    fprintf(stderr,"error setting engine write buffer array: element out of range\n");
    return 0;
  }

  return buf_entry.data;
}

void engine_alsa_detach_from_device_output_channel(engine_t* engine, int channel, int object_address){
 pair_i_v_t buf_entry;

  if (!engine->write_buffer_array) return;

  if (generic_array_get_element(engine->write_buffer_array, channel, &buf_entry) < 0){
    fprintf(stderr,"error accessing engine write buffer array: element out of range");
    return;
  }

  buf_entry.owner_object_address = -1;

  if (generic_array_set_element(engine->write_buffer_array, channel, &buf_entry) < 0){
    fprintf(stderr,"error setting engine write buffer array: element out of range");
    return;
  }

}

int engine_alsa_start(engine_t* engine){
  pthread_attr_t thread_attrib;
  int err;

  /*start sched FIFO thread in fxn polling for soundcard triggers*/
  
  if ((err = pthread_attr_init(&thread_attrib))!=0){
    printf("\nerror: could not allocate a thread attribute structure\n");
    return err;
  }
  
  if ((err = pthread_attr_setschedpolicy(&thread_attrib,SCHED_FIFO))!=0){
    printf("\nerror: could not set scheduling policy of audio thread\n");
    return err;
  }

  /*try to start a FIFO priority audio thread for low latency operation*/
  if ((err = pthread_create(&engine_thread,\
			    &thread_attrib,\
			    (void *(*)(void*))engine_alsa_poll_loop, \
			    (void*)engine)) != 0){
    
    /*tell loser what's going on*/
    /*(note that i use this term in an affectionate dual sense;*/ 
    /*see "pc losering" and note that they just lost a bid for FIFO status)*/
    printf("\nNOTE: failed to create a high priority audio thread\n");
    printf("couldn't get scheduling policy FIFO for low latency.\n");
    printf("You're probably not running Soundtank as root.\n");
    printf("Will run audio thread w/ regular priority. For low latency,\n");
    printf("run as root, use sudo, or wait 'till we implement libcap\n");
    
    /*if we can't get FIFO, try to start a regular priority audio thread*/
    if ((err = pthread_attr_setschedpolicy(&thread_attrib,SCHED_OTHER))!=0){
      printf("\nerror: could not set scheduling policy of audio thread\n");
      return err;
    }
    
    if ((err = pthread_create(&engine_thread, &thread_attrib,(void *(*)(void*))engine_alsa_poll_loop, (void*)engine))!=0){
      printf("\nerror: failed to create regular priority audio thread\n");
      return err;
    }
    
  }

  /*at this point, engine has become active*/
  engine->state = ENGINE_STATE_ACTIVE;
  
  if ((err = pthread_attr_destroy(&thread_attrib))!=0){
    printf("\nerror: could not deallocate thread attribute structure\n");
    /*still return 0 because we succeeded in starting engine*/
  }
  
  return 0;
}


int engine_alsa_stop(engine_t* engine){
  void* engine_return;

  /*send signal to stop engine*/
  struct swap_message local_msg;
  local_msg.msg_type = MSG_TYPE_STOP_ENGINE;
  if (msgsnd(msgid,&local_msg,(size_t)MSG_SIZE,0)) return -1;

  /*wait for reply message saying that engine has stopped*/
  msgrcv(msgid,&local_msg,(size_t)MSG_SIZE,MSG_TYPE_FINISHED_STOP_ENGINE,0);

  /*join engine thread*/
  if ((pthread_join(engine_thread, &engine_return))<0){
    printf("Error: error rejoining ALSA engine thread\n");
    return -1;
  }

  return 0;
}


static int wait_for_poll(snd_pcm_t *handle, struct pollfd *ufds, unsigned int count)
{
        unsigned short revents;

        while (1) {
	  poll(ufds, count, -1);
	  snd_pcm_poll_descriptors_revents(handle, ufds, count, &revents);
	  if (revents & POLLERR)
	    return -EIO;
	  if (revents & POLLOUT)
	    return 0;
        }
}

static int xrun_recovery(int err)
{
  /*record that xrun occured*/
  soundtank_engine->last_xrun = soundtank_engine->elapsed_frames;
  soundtank_engine->total_xruns++;

  if (err == -EPIPE) {    
    /* under-run */
    err = snd_pcm_prepare(playback_handle);
    if (err < 0)
      printf("Can't recovery from underrun, prepare failed: %s\n", snd_strerror(err));
    return 0;
  } else if (err == -ESTRPIPE) {
    while ((err = snd_pcm_resume(playback_handle)) == -EAGAIN)
      sleep(1);       /* wait until the suspend flag is released */
    if (err < 0) {
      err = snd_pcm_prepare(playback_handle);
      if (err < 0)
	printf("Can't recovery from suspend, prepare failed: %s\n", snd_strerror(err));
    }
    return 0;
  }
  return err;
}

void* engine_alsa_poll_loop(void* eng_ptr){
  /*adapted from poll & write loop in pcm.c, example code included w/
    ALSA lib*/
  int err,i,interleaved,poll_count,init,cptr;
  void ** bufs;
  snd_pcm_sframes_t frames_to_deliver;
  signed short *ptr;
  struct pollfd *ufds;
  engine_t* engine = (engine_t*)eng_ptr;

  bufs=0;
  ufds=0;

  if (engine->interleaved == SND_PCM_ACCESS_RW_INTERLEAVED){

    interleaved = 1;

  }else{

    interleaved = 0;

    /*for noninterleaved, copy device buffer array addresses into a
      new structure (kind of redundant)*/
    if (!(bufs = malloc(engine->num_channels * sizeof(void*)))){
      printf("\nerror: couldn't allocate memory for temporary void pointers\n");
      return 0;
    }

    for (i=0;i<engine->num_channels;++i)
      bufs[i] = ((pair_i_v_t*)(generic_array_get_element_pointer(engine->write_buffer_array,i)))->data;

  }

  /*initialize poll structures for receiving device interrupts from ALSA*/
  poll_count = snd_pcm_poll_descriptors_count(playback_handle);
  if (poll_count <= 0) {
    printf("Invalid poll descriptors count\n");
    free(bufs);
    return 0;
  }

  ufds = (struct pollfd*)malloc(sizeof(struct pollfd) * poll_count);
  if (ufds == NULL) {
    printf("Not enough memory\n");
    free(bufs);
    return 0;
  }

  if ((err = snd_pcm_poll_descriptors(playback_handle, ufds, poll_count)) < 0) {
    printf("Unable to obtain poll descriptors for playback: %s\n", snd_strerror(err));
    free(bufs);
    free(ufds);
    return 0;
  }
  
   /*actually, prepare is not needed the first time you start the device*/
    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
      fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror (err));
      /* TODO handle this error differently?*/
      free(bufs);
      free(ufds);
      return 0;
   }


   if (debug_readout) printf("going into alsa poll loop::::::\n");





   init = 1;
   while (1){

  
     if (!init) {
       err = wait_for_poll(playback_handle, ufds, poll_count);
       if (err < 0) {
	 if (snd_pcm_state(playback_handle) == SND_PCM_STATE_XRUN ||
	     snd_pcm_state(playback_handle) == SND_PCM_STATE_SUSPENDED) {
	   err = snd_pcm_state(playback_handle) == SND_PCM_STATE_XRUN ? -EPIPE : -ESTRPIPE;
	   if (xrun_recovery(err) < 0) {
	     if (rt_error_readout == RT_ERROR_READOUT_LOUD){
	       printf("Write error: %s\n", snd_strerror(err));
	       print_err_msg(SOUNDTANK_RT_ERROR,"generic rt error");
	     }
	     break;
	   }
	   init = 1;
	 } else {
	   if (rt_error_readout == RT_ERROR_READOUT_LOUD){
	     printf("Wait for poll failed\n");
	     print_err_msg(SOUNDTANK_RT_ERROR,"generic rt error");
	   }
	   break;
	 }

	 if (rt_error_readout == RT_ERROR_READOUT_LOUD){
	   printf("wait for poll failed\n");
	   print_err_msg(SOUNDTANK_RT_ERROR,"generic rt error");
	 }
       }
       
     }
     
     /*TODO get rid of frames-to-deliver variable*/
     frames_to_deliver = soundtank_engine->period_size;


     if ((err = realtime_process_function(frames_to_deliver))<0) break;

     ptr = (signed short*) soundtank_engine->buffer;

     cptr = frames_to_deliver;

     while (cptr > 0){


       /*write to hardware*/
       if (interleaved){
	 err = snd_pcm_writei(playback_handle,ptr , cptr);
       }else{
	 err = snd_pcm_writen(playback_handle,bufs , cptr);
       }
       
       if (err < 0) {
	 if (xrun_recovery(err) < 0) {
	   if (rt_error_readout == RT_ERROR_READOUT_LOUD){
	     printf("Write error: %s\n", snd_strerror(err));
	     print_err_msg(SOUNDTANK_RT_ERROR,"generic rt error");
	   }
	   break;
	 }
	 init = 1;
	 break;  /* skip one period */
       }

       if (snd_pcm_state(playback_handle) == SND_PCM_STATE_RUNNING){
	 init = 0;
       }else{
	 if (rt_error_readout == RT_ERROR_READOUT_LOUD){
	   printf("error: soundcard did not start\n");
	   print_err_msg(SOUNDTANK_RT_ERROR,"generic rt error");
	 }
	 break;
       }
       
       soundtank_engine->elapsed_frames += err;
       ptr += err * soundtank_engine->num_channels;
       /*TODO !!!! add support for noninterleaved access here*/
       cptr -= err;

       /* it is possible, that the initial buffer cannot store */
       /* all data from the last period*/

       if (cptr == 0){
	 /*in this case, the first buffer write did store all data from the last period*/
	 break;
       }

       err = wait_for_poll(playback_handle, ufds, poll_count);
       if (err < 0) {
	 if (snd_pcm_state(playback_handle) == SND_PCM_STATE_XRUN ||
	     snd_pcm_state(playback_handle) == SND_PCM_STATE_SUSPENDED) {
	   err = snd_pcm_state(playback_handle) == SND_PCM_STATE_XRUN ? -EPIPE : -ESTRPIPE;
	   if (xrun_recovery(err) < 0) {
	     if (rt_error_readout == RT_ERROR_READOUT_LOUD){
	       printf("Write error: %s\n", snd_strerror(err));
	       print_err_msg(SOUNDTANK_RT_ERROR,"generic rt error");
	     }
	     break;
	   }
	   init = 1;
	 } else {
	   if (rt_error_readout == RT_ERROR_READOUT_LOUD){
	     printf("Wait for poll failed\n");
	     print_err_msg(SOUNDTANK_RT_ERROR,"generic rt error");
	   }
	   break;
	 }
	 
       }
		


     }


   }

   /*check for normal exit code*/
   if (err != -3)
     printf("\nALSA POLL LOOP EXITED UNEXPECTEDLY!!!\n\n");

   /*stop pcm device*/
   if (snd_pcm_drop(playback_handle) < 0){
     printf("error: unable to stop ALSA pcm device, runaway train\n");
   }

   /*at this point, engine has become inactive*/
   if (snd_pcm_state(playback_handle) == SND_PCM_STATE_RUNNING){
     printf("error: unable to stop ALSA pcm device, runaway train\n");
     soundtank_engine->state = ENGINE_STATE_ACTIVE;
   }else{
     soundtank_engine->state = ENGINE_STATE_INACTIVE;
   }
  
   /*send message to admin thread saying that engine's stopped*/
   {
     struct swap_message local_msg;
     local_msg.msg_type = MSG_TYPE_FINISHED_STOP_ENGINE;
     if (msgsnd(msgid,&local_msg,(size_t)MSG_SIZE,0))
       printf("messaging error occured while trying to stop engine\n");
   }

   /*free up memory*/
   if (bufs) free(bufs);
   if (ufds) free(ufds);

   return 0;
}

void engine_alsa_print_state(){

  if (snd_pcm_state(playback_handle) == SND_PCM_STATE_RUNNING){
   printf("Active");
   soundtank_engine->state = ENGINE_STATE_ACTIVE;
  }else{
   printf("Not Active");
   soundtank_engine->state = ENGINE_STATE_INACTIVE;
  }


}
