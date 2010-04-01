/*
 * soundtank internal commands code
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





#ifndef SOUNDTANK_INTERNAL_COMMANDS_INCLUDES
#define SOUNDTANK_INTERNAL_COMMANDS_INCLUDES

/* commands.c */
int soundtank_execute_command(const char* command_line);

/*engine_commands.c*/
void soundtank_command_start(int argc, char** argv);
void soundtank_command_stop(int argc,char** argv);

/*debug.c*/
void soundtank_command_debug(int argc, char** argv);

/*stat.c*/
void soundtank_command_stat(int argc,char** argv);

/*ls.c*/
void printout_major_type(int major_type);
void printout_imp_type(int imp_type);
void rtobject_printout_info(rtobject_t* rtobj);
void soundtank_command_ls(int argc, char** argv);

/*cd.c*/
void soundtank_command_cd(int argc, char** argv);
void soundtank_command_pwd(int argc, char** argv);

/*cr.c*/
void soundtank_command_create_rtobject(int argc, char** argv);
void soundtank_command_copy_rtobject(int argc, char** argv);
void soundtank_command_free_rtobject(int argc, char** argv);

/*sv.c*/
void soundtank_command_load_rtobject(int argc, char** argv);
void soundtank_command_save_rtobject(int argc, char** argv);


/*mv.c*/
void soundtank_command_move_rtobject(int argc, char** argv);
void soundtank_command_remove_rtobject(int argc, char** argv);

/*at.c*/
void soundtank_command_attach_rtobject(int argc, char** argv);
void soundtank_command_detach_rtobject(int argc, char** argv);

/*set.c*/
void soundtank_command_set(int argc, char** argv);
void soundtank_command_activate(int argc, char** argv);
void soundtank_command_mute(int argc, char** argv);
void soundtank_command_volume(int argc, char** argv);

/*map.c*/
void soundtank_command_map(int argc, char** argv);
void soundtank_command_test(int argc, char** argv);
void soundtank_command_action(int argc, char** argv);
void soundtank_command_scale(int argc, char** argv);


#endif
