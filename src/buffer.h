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


#ifndef SOUNDTANK_BUFFER_INCLUDE
#define SOUNDTANK_BUFFER_INCLUDE

/*buffer reference flags*/
#define BUFF_FREE 1
#define BUFF_EXCLUSIVE 2
#define BUFF_SHARE 3
#define BUFF_WORKING 4

struct mono_audio_buffer{  

  sample_count_t numsamples;
  sample_t* data;

  unsigned lazy_zero : 1;

  /*TODO: remove the following 2 items*/
  /*
  unsigned scale_input_active : 1;
  float scale_input;
  */

  int address_index; /*unique name of this buffer*/
  int flags; /*BUFF_FREE,BUFF_EXCLUSIVE,BUFF_SHARE,BUFF_WORKING*/

  ll_head reference_list;

};



buffer_t* buffer_alloca(int address,sample_count_t size);
void buffer_dealloca(buffer_t* buffer);

int buffer_get_flags(const buffer_t* buffer);
int buffer_get_address(const buffer_t* buffer);
sample_t* buffer_get_data(const buffer_t* buffer);
sample_count_t buffer_get_size(const buffer_t* buffer);


void buffer_zero(buffer_t* buffer);
void buffer_not_zero(buffer_t* buffer);
int buffer_get_zero(const buffer_t* buffer);


int buffer_add_reference(buffer_t* buffer, int object_address, int flags);
int buffer_remove_reference(buffer_t* buffer, int object_address);
int buffer_check_reference(const buffer_t* buffer, int object_address);


void buffer_push(buffer_t* from, buffer_t* to);
void buffer_scale_push(buffer_t* from, buffer_t* to, float scale);

void buffer_fade_in(buffer_t* buff, sample_count_t offset, sample_count_t fade_length);
void buffer_fade_out(buffer_t* buff, sample_count_t offset, sample_count_t fade_length);

sample_t buffer_get_max(const buffer_t* buff);
sample_t buffer_get_max(const buffer_t* buff);
sample_t buffer_get_avg(const buffer_t* buff);


void buffer_debug_print(const buffer_t* buff, sample_count_t num_samples);

#endif
