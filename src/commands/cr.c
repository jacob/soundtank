/*
 * soundtank internal commands code: create/free
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


#include <stdlib.h>
#include <string.h>
#include <popt.h>

#include "../include.h"
#include "../soundtank_structs.h"




void soundtank_command_create_rtobject(int argc, char** argv){
  struct poptOption optarray[12];
  poptContext optcontext;
  signal_path_t *target_path;
  char *create_name, *create_desc, *user_path;
  int i,j,create_major_type,create_imp_type,create_imp_argc,ret_imp_argc,pass_offset;
  int disp_help, no_automap;
  const char ** ret_imp_argv;
  const char* curr_arg;

  create_name = create_desc = user_path = 0;
  create_major_type = create_imp_type = create_imp_argc = -1;
  disp_help = no_automap = pass_offset = 0;

  /*fill poptOption array, this handles option parseing*/

  if (!(optarray[0].longName = strdup("description"))) {return ;}
  optarray[0].shortName = 'd';
  optarray[0].argInfo = POPT_ARG_STRING;
  optarray[0].arg = (void*)&create_desc; 
  optarray[0].val = 2;
  optarray[0].descrip = "new object's description";
  optarray[0].argDescrip = "string";

  if (!(optarray[1].longName = strdup("path"))) {return ;}
  optarray[1].shortName = 'p';
  optarray[1].argInfo = POPT_ARG_STRING;
  optarray[1].arg = (void*)&user_path; 
  optarray[1].val = 2;
  optarray[1].descrip = "place new object in specified path";
  optarray[1].argDescrip = "string";

  if (!(optarray[2].longName = strdup("nomap"))) {return ;}
  optarray[2].shortName = 'n';
  optarray[2].argInfo = POPT_ARG_NONE;
  optarray[2].arg = (void*)&no_automap; 
  optarray[2].val = 1;
  optarray[2].descrip ="do automatically create an event map";
  optarray[2].argDescrip = "boolean";

  if (!(optarray[3].longName = strdup("help"))) {return ;}
  optarray[3].shortName = '?';
  optarray[3].argInfo = POPT_ARG_NONE;
  optarray[3].arg = (void*)&disp_help;
  optarray[3].val = 0;
  optarray[3].descrip = "Display Help Message";
  optarray[3].argDescrip = 0;

  optarray[4].longName =0;
  optarray[4].shortName = '\0';
  optarray[4].argInfo = 0;
  optarray[4].arg = 0; 
  optarray[4].val = 0;


  /*parse poptOption array*/

  optcontext = poptGetContext("new_rtobject",argc,(const char**)argv,optarray,0);
  j=0;
  while ((i=poptGetNextOpt(optcontext)) > 0){
    ++j;
    if ((i<0)&&(i!=-1)){
      printf("\n invalid option %s\n",argv[(j*2)+1]); /*TODO doesn't work*/
      return;
    }
  }

  /*check for help option*/
  if (disp_help){

      printf("Useage: cr [OPTIONS] major-type name [imp_type] [imp args] ...\n");
      printf("Explanation: cr creates new rtobjects in soundtank\n");
      printf("\n");
      printf("    [OPTIONS] = -n : no automatic map creation\n");
      printf("    [OPTIONS] = -p path : create in specified path\n");
      printf("\n");
      printf("    [major-type] = 'src' aka 'source', 'flt' aka 'filter'\n");
      printf("    'path' aka 'signal_path', 'out' or 'in', 'extern_out' or 'extern_in'\n");
      printf("\n");
      printf("    [name] = the name for the newly created object\n");
      printf("\n");
      printf("    [imp_type] = soundtank internal implementation type, usually defaults ok\n");
      printf("\n");
      printf("    [imp args] = the implementation arguments to pass to imp object,\n");
      printf("    examples include ladspa plugin names like 'sine_fcac' 'sin_osc' ...\n");
      printf("\n");
      printf("    LADSPA Plugin Arg Format: [plugin-label] | [-i|--id plugin-id#] [-o] \n");
      printf("    The plugin label is a string, -i or --id must be followed by a valid plugin id #\n");
      printf("    The -o option means that plugin's output destructively overwrites channel data\n");
      printf("\n");
      return;

  }

  



  /*handle the remaining arguments which are core data and imp data */
  if (!(ret_imp_argv = poptGetArgs(optcontext))){
    printf("ERROR: could not parse non-optional create arguments\n");
    return;
  }
  
  /*count how many non-option args*/
  for (i=0;;++i){
    curr_arg = ret_imp_argv[i];
    if (!curr_arg) break;
    if (debug_readout) printf("arg %d=%s.\n",i,curr_arg);
  }
  ret_imp_argc = i;

  /*major type & name must be specified*/
  if (ret_imp_argc < 2){
    printf("create error: not enough arguments, must specify major type and name\n");
    return;
  }
  
  /*get major type*/
  if ((create_major_type = string_to_rtobject_major_type(ret_imp_argv[0])) < 0){
    printf("create error: could not parse major type\n");
    return;
  }

  /*get name*/
  if (!(create_name = strdup(ret_imp_argv[1]))){
    printf("create error: memory error\n");
    return;
  }
  
  /*verify desc*/
  if (!create_desc) create_desc = strdup("\0");

  /*if an argument after the major type exists and is an imp type, we use it, otherwise we guess*/
  if ((ret_imp_argc > 2)&&\
      ((create_imp_type = string_to_rtobject_imp_type(ret_imp_argv[2])) >= 0)){
    
    /*when passing imp args, skip 4 args: 'cr' major-type name imp-type*/
    pass_offset = 4;

  }else{
    
    /*get default imp type for major type*/
    if ((create_imp_type = major_type_to_default_imp_type(create_major_type)) < 0){
      printf("create error: could not guess imp type, you must supply imp type\n");
      return;
    }

    /*when passing imp args, skip 3 args: 'cr' major-type name*/
    pass_offset = 3;

  }

  /*find the target path*/
  if (!user_path){

    target_path = curr_path;
    
  }else{
    rtobject_t *target_path_rtobj;

    /*when passing imp args, also skip 2 more args: '-p' pathname*/
    pass_offset += 2;

    if (!(target_path_rtobj = get_rtobject_from_path(user_path))){
      printf("create error: could not find destination path %s\n", user_path);
      return;
    }

    if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(target_path_rtobj)){
      printf("create error: destination path, %s, is not a signal path\n", user_path);
      return;
    }

    target_path = (signal_path_t*)target_path_rtobj->imp_struct;

  }

  /*when passing imp args, also skip the '-n' argument*/
  if (no_automap) pass_offset += 1;

  /*free poptContext when finished parseing*/
  poptFreeContext(optcontext);

  if ((i = create_rtobject(create_major_type,create_imp_type,(argc - pass_offset),(const char**)&argv[pass_offset], create_name, create_desc, target_path))<0){
    printf("ERROR occured while trying to create rtobject\n");
    return;
  }

  /*create an event map automatically unless user tells us not to*/
  if (!no_automap){
    rtobject_t *new_obj;

    if (!(new_obj = get_rtobject_from_address(i))){
      printf("\n could not find new rtobject, no event map will be created\n");
      return;
    }

    if (rtobject_create_auto_map(new_obj) < 0){
      printf("error occured while trying to automatically create event map for new rtobject\n");
      return;
    }
    
  }

  /*remake the process list to make object live*/
  if ((remake_process_list()) < 0){
    printf("create rtobject error: failed while remaking process list, object not live\n");
  }

}

void soundtank_command_copy_rtobject(int argc, char** argv){
  rtobject_t* rtobj;
  signal_path_t* target_path;
  char* new_name;
  int i,diff_path;

  /*cp rtobject target*/
  if (argc < 3){
    printf("copy error: not enough arguments\n");
    return;
  }

  /*find object*/
  if (!(rtobj = get_rtobject_from_path(argv[1]))){
    printf("copy error: could not find rtobject %s\n",argv[1]);
    return;
  }

  /*parse target path & new name*/
  /*see if copy is in a different signal path*/
  diff_path = 0;
  for (i=0;i<strlen(argv[2]);++i)
    if (argv[2][i] == '/') diff_path = 1;
  
  if (!diff_path){
    /*easy case: same path*/
    
    target_path = rtobject_get_parent_path(rtobj);
    new_name = strdup(argv[2]);
    
  }else{
    /*more work case: different path*/
    int new_name_len;
    char* to_path_name;
    rtobject_t* to_path_rtobj;
    
    /*how long is new name at end?*/
    for (i = strlen(argv[2]) - 1;i>=0;--i)
      if (argv[2][i] == '/') break;
    new_name_len = strlen(argv[2]) - 1 - i;
    
    /*copy new name*/
    if (!(new_name = strdup(&argv[2][i+1]))){
      printf("copy error: memory error\n");
      return;
    }
    
    /*copy new path name*/
    if (!(to_path_name = (char*)malloc(i+1))){
      printf("copy error: memory error\n");
      free(new_name);
      return;
    }     
    strncpy(to_path_name,argv[2],i);
    to_path_name[i] = '\0';
    
    /*find target path rtobject*/
    if (!(to_path_rtobj = get_rtobject_from_path(to_path_name))){
      printf("mover error: could not find destination path %s\n",to_path_name);
      free(new_name);
      free(to_path_name);
      return;
    }
    
    /*verify new path is a path*/
    if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH != rtobject_get_major_type(to_path_rtobj)){
      printf("move error: destination path %s does not exist\n",to_path_name);
      free(new_name);
      free(to_path_name);
      return;
    }
    
    /*get target path*/
    target_path = (signal_path_t*)to_path_rtobj->imp_struct;
   
  } 
    
  /*do soundtank copy call*/
  if ((create_rtobject_dup(rtobj, new_name, target_path)) < 0){
    printf("copy error: failed attempt to duplicate rtobject\n");
      return;
  }

  /*remake the process list to make object live*/
  if ((remake_process_list()) < 0){
    printf("create rtobject error: failed while remaking process list, object not live\n");
  }
  
}

void recursive_rtobject_delete(rtobject_t* rtobj){
  
  /*if it's a signal path, delete all member objects before deleting rtobj*/
  if (rtobject_get_major_type(rtobj) == RTOBJECT_MAJOR_TYPE_SIGNAL_PATH){
    node_t* temp_node;
    
    /*note that sigpath get node fxn is not zero based for some strange reason*/
    while ((temp_node = signal_path_get_node_from_position((signal_path_t*)rtobj->imp_struct, 1))){
      
      recursive_rtobject_delete((rtobject_t*)temp_node->data);

    }

  }

  /*once we've eliminated all member rtobjects we destroy rtobj*/
  destroy_rtobject(rtobj);
   
}

void soundtank_command_free_rtobject(int argc, char** argv){
  rtobject_t* rtobj;
  int recurse, i;

  /*check for help*/
  for (i=0;i<argc;++i){
    if ((!strcmp(argv[i],"-h"))||(!(strcmp(argv[i],"--help")))){
      printf("useage: rm [OPTIONS] rtobject\n");
      printf("explanation: rm removes an rtobject from soundtank\n");
      printf("OPTIONS: -r or -R for recursive deletion of signal path member objects\n");
      return;
    }
  }

  /*need at least an object */
  if (argc < 2){
    printf("free error: not enought arguments\n");
    return;
  }

  /*check if recursive delete desired*/
  recurse = 0;
  for (i=0;i<argc;++i){
    if ((!(strcmp(argv[i],"-r")))||(!(strcmp(argv[i],"-R")))){
      recurse = 1;
      break;
    }
  }

  /*find object*/
  if (!(rtobj = get_rtobject_from_path(argv[1 + recurse]))){
    printf("free error: could not find rtobject %s\n",argv[1]);
    return;
  }

  if (!recurse){
    
    /*easy case, no recursion*/
    destroy_rtobject(rtobj);

  }else{

    /*must do a depth-first recursion because you can't delete a signal path with members*/
    /*NOTE: this fxn is locally defined right above here*/
    recursive_rtobject_delete(rtobj);
    
  }

  /*remake the process list to enact deletion*/
  if ((remake_process_list()) < 0){
    printf("delete rtobject error: failed while remaking process list, object still live, DATA CORRUPTION!\n");
  }

}
