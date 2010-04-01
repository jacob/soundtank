/*
 * soundtank internal commands code: attach/detach data ports
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


static int get_next_input_port(rtobject_t* rtobj, int curr_port){
  int pos;
  for (pos=curr_port;pos<rtobject_get_data_port_list_size(rtobj);++pos){
    if (data_port_get_input(rtobject_get_data_port(rtobj,pos)))
      return pos;
  }
  return -1;
}

static int get_next_output_port(rtobject_t* rtobj, int curr_port){
  int pos;
  for (pos=curr_port;pos<rtobject_get_data_port_list_size(rtobj);++pos){
    if (!(data_port_get_input(rtobject_get_data_port(rtobj,pos))))
      return pos;
  }
  return -1;
}

static void intelligent_attach_ports(rtobject_t* from, rtobject_t* to){
  int from_port, to_port;
  from_port = to_port = 0;
  
  /*need at least one suitable target port*/
  if ((to_port = get_next_input_port(to,to_port)) < 0){
    printf("attach error: no input ports to attach to\n");
    return;
  }

  while ((from_port = get_next_output_port(from,from_port)) >= 0){

    if ((connect_data_ports(from, from_port++, to, to_port)) < 0){
      printf("attach error: failed attempt to connect from port %d to port %d\n", from_port, to_port);
      return;
    }

    if ((get_next_input_port(to,to_port+1)) >= 0)
      to_port = get_next_input_port(to,to_port+1);

  }

}


void soundtank_command_attach_rtobject(int argc, char** argv){
  int i;
  rtobject_t* rtobj;
  rtobject_t* target_obj;

  /*handle help message*/
  for (i=0;i<argc;++i){
    if ((!strcmp(argv[i],"-h"))||(!(strcmp(argv[i],"--help")))){
      printf("explanation: attach is used to attach a data port to a channel\n");
      printf("explanation: attach can also attach 2 data ports; it automatically creates a channel\n");
      printf("useage: attach rtobject [port#] target [port#] : attach port(s) to another rtobject\n");
      printf("useage: attach rtobject [port#] chan|-p|* chan# : attach port(s) to parent path channel, creates new channel if chan# doesn't exist\n");
      printf("useage: attach rtobject port# new : attach port to new parent path channel\n");
      printf("note: if no port is given, all ports will be attached\n");
      return;
    }
  }

  /*minimum 3 args: 'attach' rtobject target-address*/
  if (argc < 3){
    printf("attach error: not enough arguments\n");
    return;
  }
  
  /*find the object*/
  if (!(rtobj = get_rtobject_from_path(argv[1]))){
    printf("attach error: could not find rtobject %s\n",argv[1]);
    return;
  }

  if (argc == 3){
    /*'attach' rtobject target*/
    /*intelligently connect entire object to target*/
        
    if (!(target_obj = get_rtobject_from_path(argv[2]))){
      printf("set target error: couldn't find target object %s\n",argv[2]);
      return;
    }
    
    intelligent_attach_ports(rtobj, target_obj);



  }else if ((argc == 5)){
    signal_path_t* parent_path;
    int connect_port, target_port;
    ll_head results;
    
    results=0;

    /*parse specifier of port to be connected*/
    if (rtobject_search_data_ports(rtobj, argv[2], SEARCH_EXACT, &results) <= 0){
      printf("att error: unknown data port, %s\n", argv[2]);
      return;
    }
    connect_port = *((int*)results->data);
    rtobject_free_search_results(&results);


    /*determine target object and port*/
    /*these options mean attach to parent path internal channel*/
    if ( (!(strcmp(argv[3],"chan")))||(!(strcmp(argv[3],"-p")))||(!(strcmp(argv[3],"*")))){
      /*'attach' rtobject port parent-path local_channel*/
      
      /*find parent sigpath*/
      if (!(parent_path = rtobject_get_parent_path(rtobj))){
	printf("attach error: rtobject has no parent path\n");
	return;
      }
      
      if (!(target_obj = get_rtobject_from_address(signal_path_get_owner_rtobject_address(parent_path)))){
	printf("set target error: couldn't find parent path's rtobject %s\n",argv[3]);
	return;
      }
    
      /*target port is target channel*/
      target_port = atoi(argv[4]);

    }else{
      /*if not shorthand for parent path then lookup target object*/

      if (!(target_obj = get_rtobject_from_path(argv[3]))){
	printf("set target error: couldn't find target object %s\n",argv[3]);
	return;
      }

      /*parse specifier of target port*/
      if (rtobject_search_data_ports(target_obj, argv[4], SEARCH_EXACT, &results) <= 0){
	printf("att error: unknown target data port, %s\n", argv[4]);
	return;
      }
      target_port = *((int*)results->data);
      rtobject_free_search_results(&results);    
      
    }

    if ((connect_data_ports(rtobj, connect_port,target_obj ,target_port)) < 0){
      printf("attach error: failed attempt to set target\n");
      return;
    }

  }else if ((argc == 4)&&((!(strcmp(argv[2],"chan")))||(!(strcmp(argv[2],"-p")))||(!(strcmp(argv[2],"*"))))){
    signal_path_t* parent_path;
    int target_port;
    
    /*attaching all ports to parent channel x*/
 
    /*find parent sigpath*/
    if (!(parent_path = rtobject_get_parent_path(rtobj))){
      printf("attach error: rtobject has no parent path\n");
      return;
    }
    
    if (!(target_obj = get_rtobject_from_address(signal_path_get_owner_rtobject_address(parent_path)))){
      printf("set target error: couldn't find parent path's rtobject %s\n",argv[3]);
      return;
    }

    /*find target channel*/
    if (!(string_is_number(argv[3]))){
      printf("attach error: parent channel must be an integer, you entered %s\n", argv[3]);
      return;
    }
    target_port = atoi(argv[3]);

    /*do the attaching*/
    for (i=0;i<rtobject_get_data_port_list_size(rtobj);++i){

      if ((connect_data_ports(rtobj, i, target_obj, target_port)) < 0){
	printf("attach error: failed attempt to attach port %d\n", i);
	return;
      }

    } 

  }else if ((argc == 4)&&(!(strcmp(argv[3],"new")))){
    int connect_port, chan_index;
    signal_path_t* parent_path;
    channel_t* chan;
    
    /*'attach' rtobject port 'new'*/
 
    /*find parent sigpath*/
    if (!(parent_path = rtobject_get_parent_path(rtobj))){
      printf("attach error: rtobject has no parent path\n");
      return;
    }
   
    connect_port = atoi(argv[2]);

    /*make new channel*/
    if ((signal_path_is_master_path(parent_path))){
      if (!(chan = signal_path_create_new_channel(parent_path,CHANNEL_SCOPE_GLOBAL))){
	printf("attach error: couldn't make new channel\n");
	return;
      }
    }else{
      if (!(chan = signal_path_create_new_channel(parent_path,CHANNEL_SCOPE_LOCAL))){
	printf("attach error: couldn't make new channel\n");
	return;
      }
    }
   
    if ((rtobject_attach_port_to_channel(rtobj, connect_port, chan)) < 0){
      printf("attach error: attach failed\n");
      return;
    }

  }else{
    printf("attach error: not enough arguments for port specific connection\n");
    return;
  }
  
  /*remake process list*/
  if ((remake_process_list()) < 0){
    printf("error occured while remaking process list\n");
  }

}



void soundtank_command_detach_rtobject(int argc, char** argv){
  rtobject_t* rtobj;
  signal_path_t* parent_path;
  int i;

  /*minimum 2 args: 'detach' rtobject*/
  if (argc < 2){
    printf("detach error: not enough arguments\n");
    return;
  }
  
  /*find the object*/
  if (!(rtobj = get_rtobject_from_path(argv[1]))){
    printf("detach error: could not find rtobject %s\n",argv[1]);
    return;
  }

  /*sanity check: verify object is in a path*/
  if (!(parent_path = rtobject_get_parent_path(rtobj))){
    printf("detach error: couldn't find object's parent path\n");
    return;
  }

  if (argc == 2){
    
    for (i=0;i<rtobject_get_data_port_list_size(rtobj);++i){

      if ((rtobject_detach_port(rtobj, i)) < 0){
	printf("detach error: failed attempt to disconnect port %d\n",i);
	if (!i) return;
      }

    }    
    
  }else if (argc == 3){
    /*just detach one port*/
    ll_head results;
    
    results=0;

    /*parse specifier of port to be connected*/
    if (rtobject_search_data_ports(rtobj, argv[2], SEARCH_EXACT, &results) <= 0){
      printf("att error: unknown data port, %s\n", argv[2]);
      return;
    }
    i = *((int*)results->data);
    rtobject_free_search_results(&results);

    if ((rtobject_detach_port(rtobj, i)) < 0){
	printf("detach error: failed attempt to disconnect port %d\n",i);
	return;
    }

  }else{
    printf("too many arguments\n");
    return;
  }

  /*remake process list*/
  if ((remake_process_list()) < 0){
    printf("error occured while remaking process list\n");
  }

}
