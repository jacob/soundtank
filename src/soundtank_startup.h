/*
 *soundtank - application startup
 *
 *Copyright Jacob Robbins 2003
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

#ifndef SOUNDTANK_SOUNDTANK_STARTUP
#define SOUNDTANK_SOUNDTANK_STARTUP

/*soundtank_startup*/
void soundtank_cleanup();
int soundtank_init(int argc, const char** argv);

int soundtank_make_default_extern_outs();

int soundtank_get_command();
int soundtank_load_command_history();
int soundtank_save_command_history(int numlines);

char** soundtank_completion (const char* text, int start, int end);
char* rtobject_name_generator(const char* text, int state);


/*TODO: remove the following 2 fxns*/
void reprint_command_line();
void print_err_msg(int err_code, char* msg);




#endif
