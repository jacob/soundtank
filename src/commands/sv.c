/*
 * soundtank internal commands code: save/load
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


void soundtank_command_load_rtobject(int argc, char** argv){
  int i, pathname_len;
  char *pathname;

  /*handle help message*/
  for (i=0;i<argc;++i){
    if ((!strcmp(argv[i],"-h"))||(!(strcmp(argv[i],"--help")))){
      printf("explanation: load creates an rtobject from an xml file\n");
      printf("useage: load|ld  pathname  : (currently must use absolute pathname)\n");
      return;
    }
  }

  /*need at least a pathname*/
  if (argc < 2){
    printf("load error: not enough arguments\n");
    return;
  }

  /*expand the pathname to an absolute pathname*/
  if (argv[1][0] == '~'){
    char* ret;
  
    /*must expand home directory symbol (~) to get absolute pathname*/
    if (!(ret = getenv("HOME"))){
      printf("xml load error: couldn't get home directory from environment\n");
      return;
    }

    /*find how long pathname must be*/
    pathname_len = strlen(ret) + strlen(argv[1]); /* +1 for null -1 for ~*/

    if (!(pathname = (char*)malloc(pathname_len * sizeof(char)))){
      printf("xml load error: memory error\n");
      return;
    }
    
    strcpy(pathname, ret);
    strcpy(&pathname[strlen(ret)], &argv[1][1]);

  }else if (argv[1][0] == '/'){

    /*user has already supplied absolute pathname*/
    pathname = argv[1];

  }else{
    char* ret;

    /*must expand current directory to get absolute pathname*/
    if (!(ret = getenv("PWD"))){
      printf("xml load error: couldn't get current directory from environment\n");
      return;
    }

    /*find how long pathname must be*/
    pathname_len = strlen(ret) + strlen(argv[1]) + 2; /* +1 for null
                                                         +1 for '/'*/

    if (!(pathname = (char*)malloc(pathname_len * sizeof(char)))){
      printf("xml load error: memory error\n");
      return;
    }
    
    strcpy(pathname, ret);
    pathname[strlen(ret)] = '/';
    strcpy(&pathname[strlen(ret) + 1], argv[1]);
      
  }

  if (debug_readout) printf("loading %s\n", pathname);

  if (load_soundtank_xml_file(pathname) < 0){
    printf("xml load error: failed to load xml file\n");
    return;
  }

}
 

void soundtank_command_save_rtobject(int argc, char** argv){
  int i, file_name_len;
  rtobject_t* obj;
  char* file_name;

  /*handle help message*/
  for (i=0;i<argc;++i){
    if ((!strcmp(argv[i],"-h"))||(!(strcmp(argv[i],"--help")))){
      printf("explanation: save stores an rtobject to an xml file\n");
      printf("useage: save|sv rtobject pathname: (currently must use absolute path)\n");
      return;
    }
  }
 
  /*need at least an object, and a pathname*/
  if (argc < 3){
    printf("save error: not enough arguments\n");
    return;
  }

  /*parse file name*/
  /*expand the filename to an absolute pathname*/
  if (argv[2][0] == '~'){
    char* ret;
  
    /*must expand home directory symbol (~) to get absolute pathname*/
    if (!(ret = getenv("HOME"))){
      printf("xml load error: couldn't get home directory from environment\n");
      return;
    }

    /*find how long file_name must be*/
    file_name_len = strlen(ret) + strlen(argv[2]); /* +1 for null -1 for ~*/

    if (!(file_name = (char*)malloc(file_name_len * sizeof(char)))){
      printf("xml load error: memory error\n");
      return;
    }
    
    strcpy(file_name, ret);
    strcpy(&file_name[strlen(ret)], &argv[2][1]);

  }else if (argv[2][0] == '/'){

    /*user has already supplied absolute pathname*/
    file_name = argv[2];

  }else{
    char* ret;

    /*must expand current directory to get absolute pathname*/
    if (!(ret = getenv("PWD"))){
      printf("xml load error: couldn't get current directory from environment\n");
      return;
    }

    /*find how long pathname must be*/
    file_name_len = strlen(ret) + strlen(argv[2]) + 2; /* +1 for null
                                                         +1 for '/'*/

    if (!(file_name = (char*)malloc(file_name_len * sizeof(char)))){
      printf("xml load error: memory error\n");
      return;
    }
    
    strcpy(file_name, ret);
    file_name[strlen(ret)] = '/';
    strcpy(&file_name[strlen(ret) + 1], argv[2]);
      
  }
  
  /*find the object*/
  if (!(obj = get_rtobject_from_path(argv[1]))){
    printf("save error: could not find rtobject %s\n",argv[1]);
    return;
  }

  /*save to file*/
  if (rtobject_save_to_file(obj, file_name) < 0){
    printf("save error: could not save to file\n");
    return;
  }


}

