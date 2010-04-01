/*
 * realtime processing function code
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


#ifndef SOUNDTANK_PROCESS_INCLUDE
#define SOUNDTANK_PROCESS_INCLUDE

/*WARNING: DO NOT CALL THIS FUNCTION!!!*/
/*WARNING: THIS FUNCTION SHOULD NOT BE CALLED EXCEPT BY AUDIO ENGINE CODE*/
/*WARNING: UI THREADS SHOULD ONLY CHANGE THE PROCESS LIST AND OTHER STRUCTURES USED BY THE PROCESS FXN BY SENDING SYSTEM V IPC MESSAGES TO THE RT THREAD*/



/* _the_ process function*/
int realtime_process_function(sample_count_t numframes);




#endif
