/*
 * mono audio buffer code
 *
 * Copyright 2003 Jacob Robbins
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include.h"


buffer_t* buffer_alloca(int address, sample_count_t size){

  buffer_t* newbuf = (buffer_t*)malloc(sizeof(buffer_t));
  if (!newbuf) return 0;

  /*the internal sample data type is defined in soundtank_types.h, usually float*/
  newbuf->data = (sample_t*)malloc(size*sizeof(sample_t));
  if (!newbuf->data) {free(newbuf); return 0;}

  /*memset new buffer to zero only so debugger won't complain about unitialized data*/
  memset( (void*)newbuf->data, 0, size*sizeof(sample_t));


  newbuf->numsamples = size;
  newbuf->lazy_zero = 0;
  newbuf->address_index = address;
  newbuf->flags = BUFF_FREE;
  newbuf->reference_list = 0;
  
  return newbuf;
}


void buffer_dealloca(buffer_t* buffer){

  if (!buffer) return;

  if (buffer->data) free(buffer->data);
  buffer->data = 0;

  while (buffer->reference_list)  {
    free((int*)buffer->reference_list->data);
    ll_remove(buffer->reference_list,&buffer->reference_list);
  }

  free(buffer);
}

int buffer_get_flags(const buffer_t* buffer) {return buffer->flags;}

int buffer_get_address(const buffer_t* buffer) {return buffer->address_index;}

sample_t* buffer_get_data(const buffer_t* buffer) {return buffer->data;}

sample_count_t buffer_get_size(const buffer_t* buffer) {return buffer->numsamples;}


void buffer_zero(buffer_t* buffer){

  /*hmmmm*/
  /*nobody use this in realtime, ok, just set lazy_zero = 1*/
  /* memset((void*)buffer->data,0,buffer->numsamples * sizeof(sample_t));*/

  buffer->lazy_zero =  0x1;

}

void buffer_not_zero(buffer_t* buffer){

  buffer->lazy_zero = 0;
 
}

int buffer_get_zero(const buffer_t* buffer){
  return buffer->lazy_zero;
}

int buffer_add_reference(buffer_t* buffer, int object_address, int flags){
  
  if ((buffer->flags == BUFF_EXCLUSIVE)||((buffer->flags == BUFF_SHARE)&&(flags == BUFF_EXCLUSIVE))){
    return -1;
  }else{
    int* address;
    address = (int*)malloc(sizeof(int));
    if (!address) return -1;
    (*address) = object_address;
    ll_append(&buffer->reference_list,(void*)address);
    buffer->flags = flags;
  }
  return 0;
}

int buffer_remove_reference(buffer_t* buffer, int object_address){

  node_t* temp_node;
  if (!buffer->reference_list) return -1;
  for (temp_node = buffer->reference_list;temp_node;temp_node=temp_node->next){
    if ( *((int*)temp_node->data) == object_address){
      ll_remove(temp_node,&buffer->reference_list);
      if (!buffer->reference_list) buffer->flags = BUFF_FREE;
      return 0;
    }
  }
  return -1;
}

int buffer_check_reference(const buffer_t* buffer, int object_address){
  node_t* temp_node;
  if (!buffer->reference_list) return 0;
  for (temp_node=buffer->reference_list;temp_node;temp_node=temp_node->next)
    if (*((int*)temp_node->data) == object_address) return 1;

  return 0;
}

void buffer_push(buffer_t* from, buffer_t* to){
  sample_count_t i;

  if ((!from)||(!to)) return;

  if (from->lazy_zero) return;

  if (to->lazy_zero){

    memcpy(to->data, from->data,from->numsamples*sizeof(sample_t));

    to->lazy_zero = 0;
    
  }else{
    
    for (i=0;i<from->numsamples;++i)
      to->data[i] += from->data[i];

  }


}

void buffer_scale_push(buffer_t* from, buffer_t* to, float scale){
  sample_count_t i;

  if ((!from)||(!to)) return;

  if (from->lazy_zero) return;

  if (to->lazy_zero){
    
    for (i=0;i<from->numsamples;++i)
      to->data[i] = (sample_t) scale * from->data[i];

    to->lazy_zero = 0;
    
  }else{

    for (i=0;i<from->numsamples;++i)
      to->data[i] += (sample_t) scale * from->data[i];

  }


}

void buffer_fade_in(buffer_t* buff, sample_count_t offset, sample_count_t fade_length){
  sample_count_t i;

  if (offset > buffer_get_size(buff))
    offset = buffer_get_size(buff);

  if ((offset + fade_length) > buffer_get_size(buff)){
    offset = 0;
    fade_length = buffer_get_size(buff);
  }

  for (i=0;i<offset;++i) buff->data[i] = 0;

  for (i=offset; i < (offset + fade_length); ++i) 
    buff->data[i] = buff->data[i] * ((double)i / (double)(offset + fade_length));

}

void buffer_fade_out(buffer_t* buff, sample_count_t offset, sample_count_t fade_length){
  sample_count_t i;

  if (offset > buffer_get_size(buff))
    offset = buffer_get_size(buff);

  if ((offset + fade_length) > buffer_get_size(buff)){
    offset = 0;
    fade_length = buffer_get_size(buff);
  }

  for (i=(offset+fade_length);i<buffer_get_size(buff);++i) buff->data[i] = 0;

  for (i=offset; i < (offset + fade_length); ++i) 
    buff->data[i] = buff->data[i] * ((double)((offset + fade_length - i) / (double)(offset + fade_length)));
				     
}


sample_t buffer_get_max(const buffer_t* buff){
  sample_t max;
  sample_count_t i;

  max = buff->data[0];

  for (i=0;i<buff->numsamples;++i)
    max =  max > buff->data[i] ? max : buff->data[i] ; 
    
  return max;
}

sample_t buffer_get_min(const buffer_t* buff){
  sample_t min;
  sample_count_t i;

  min = buff->data[0];

  for (i=0;i<buff->numsamples;++i)
    min =  min > buff->data[i] ? min : buff->data[i] ; 

  return min;
}

sample_t buffer_get_avg(const buffer_t* buff){
  sample_t avg;
  sample_count_t i;

  avg = 0;

  for (i=0;i<buff->numsamples;++i)
    avg +=  buff->data[i]; 

  return (avg / (sample_t)buff->numsamples );
}



void buffer_debug_print(const buffer_t* buff, sample_count_t num_samples){
  int i;
  node_t* temp_node;

  /*make sure we don't over step buffer array size*/
  if (num_samples > buffer_get_size(buff)){
    num_samples = buffer_get_size(buff);
    printf("buffer debug print err: buffer has incorrect size\n");
  }

  /*print buffer name*/
  printf("\n   buffer %d : ", buffer_get_address(buff));

  /*recorded size*/
  printf(" size %ld : ", buff->numsamples);

  /*buffer type*/
  switch (buff->flags){
  case BUFF_FREE: 
    printf("free buffer ");
    break;
  case BUFF_SHARE:
    printf("shared buffer ");
    break;
  case BUFF_EXCLUSIVE:
    printf("exclusive buffer ");
    break;
  case BUFF_WORKING:
    printf("working buffer ");
    break;
  }

  /*show buffer's reference list*/
  printf("used by: ");
  for (temp_node=buff->reference_list;temp_node;temp_node=temp_node->next)
    printf("%d,  ",*((int*)temp_node->data));

  /*show max, min & avg values*/
  printf("max: %.3f  ", buffer_get_max(buff));
  printf("min: %.3f  ", buffer_get_min(buff));
  printf("avg: %.3f  ", buffer_get_avg(buff));

  /*see if buffer is zeroed using lazy zero field*/
  if (buffer_get_zero(buff)){
    printf("\n   zeroed");
    return;
  }
  
  /*show buffer data location*/
  printf("\n   data %p   ",buff->data);

  /*show data values if requested*/
  if (num_samples > 0){

    printf("\n   values :  ");
    /*if buffer has data, printout first num_samples samples*/
    for (i=0;i<num_samples;++i)
      printf("%.5f  ",buff->data[i]);
    
    printf("\n");
    
  }
}
