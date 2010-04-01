/*
 * xml file load & save code
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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <iconv.h>

#include "include.h"
#include "soundtank_structs.h"


int load_soundtank_xml_file(const char* pathname){
  xmlDocPtr doc;
  xmlNodePtr curr, tmp;
  rtobject_t* curr_rtobj;
  ll_head to_load_list;
  node_t* temp_node;
  char *rdout;
  int curr_rtobj_imp_type;

  curr_rtobj = 0;
  to_load_list = 0;

  /*open file as XML document*/
  if (!(doc = xmlParseFile(pathname))){
    if (errno == ENOENT)
      printf("requested file does not exist\n");
    fprintf(stderr,"XML Document not parsed successfully: %s \n", pathname);
    return -1;
  }

  /*validate XML document root node*/
  curr = xmlDocGetRootElement(doc);
	
  if (curr == NULL) {
    fprintf(stderr,"empty document\n");
    xmlFreeDoc(doc);
    return -1;
  }
	
  if (strcmp((char*)curr->name,"SoundtankCollection")){
    fprintf(stderr,"document of the wrong type, root node != SoundtankCollection\n");
    xmlFreeDoc(doc);
    return -1;
  }else{
    if (debug_readout) printf("have opened a saved Soundtank xml file\n");
  }

  /*make linked list of all RTObjects in the Soundtank Collection*/
  for (curr=curr->children;curr;curr=curr->next){

    if (!(strcmp((char*)curr->name,"RTObject"))){

      if (!(ll_append(&to_load_list,(void*)curr))){
	printf("load error: memory error\n");
      }

    }else if (!(strcmp((char*)curr->name,"ExecCommand"))){

      /*Version 1.0 hack, ExecCommand nodes to execute commands in Soundtank*/
      if (!(rdout = xmlGetProp(curr, "Command"))){
	printf("load error: couldn't read ExecCommand node\n");
	break;
      }

      if (debug_readout) printf("executing command %s\n", rdout);
      if ((soundtank_execute_command(rdout)) < 0){
	printf("error occured executing command: %s\n", rdout);
	break;
      }

    }

  }

  /*go through linked list trying to load all RTObjects*/
  for (temp_node=to_load_list;temp_node;temp_node=temp_node->next){

    /*TODO: pretty up this hack*/
    /*don't try to load JACK externs into an ALSA engine & vice versa*/
    if (!(rdout = xmlGetProp((xmlNodePtr)temp_node->data, "ImpType"))){
      printf("xml error: couldn't read attribute ImpType\n");
      break;
    }

    if ((curr_rtobj_imp_type = string_to_rtobject_imp_type(rdout)) < 0){
      printf("xml error: invalid ImpType attribute, %s\n", rdout);
      break;
    }
    
    if ((engine_get_method(soundtank_engine) == ENGINE_METHOD_ALSA)&&\
	((curr_rtobj_imp_type == RTOBJECT_IMP_TYPE_JACK_EXTERN_OUT)||\
	 (curr_rtobj_imp_type == RTOBJECT_IMP_TYPE_JACK_EXTERN_IN)))
      break;

    if ((engine_get_method(soundtank_engine) == ENGINE_METHOD_JACK)&&\
	((curr_rtobj_imp_type == RTOBJECT_IMP_TYPE_ALSA_EXTERN_OUT)||\
	 (curr_rtobj_imp_type == RTOBJECT_IMP_TYPE_ALSA_EXTERN_IN)))
      break;

    /*try to load RTObject node*/
    if (!(curr_rtobj = rtobject_load_from_xml((xmlNodePtr)temp_node->data,to_load_list))){ 
      printf("xml load error: failed to load an RTObject, continuing\n");
    }

  }

  /*go through linked list again trying to load all Event Maps*/
  for (temp_node=to_load_list;temp_node;temp_node=temp_node->next){
    curr = (xmlNodePtr)temp_node->data;

    /*find current rtobject*/
    for (tmp = curr->children; tmp; tmp = tmp->next){

      if ((tmp->name)&&(!strcmp((char*)tmp->name,"Loaded"))){

	if (!(rdout = xmlGetProp(tmp, "Path"))){
	  printf("xml load error: list of loaded rtobjects has been corrupted\n");
	  return -1;
	}

	if (!(curr_rtobj = get_rtobject_from_path(rdout))){
	  printf("xml load error: list of loaded rtobjects has been corrupted\n");
	  return -1;
	}
      
      }

    }

    /*load event maps for current rtobject*/
    if (curr_rtobj){
      for (tmp = curr->children; tmp; tmp = tmp->next){
	
	if ((tmp->name)&&(!strcmp((char*)tmp->name,"EventMap"))){
	  event_map_t* new_map;
	  
	  if (!(new_map = event_map_load_from_xml(&tmp, curr_rtobj))){
	    printf("xml load error: unable to load stored map, continuing\n");
	  }else{
	    
	    if (rtobject_append_map(curr_rtobj, new_map) < 0){
	      printf("xml load error: unable to attach loaded map, continuing\n");
	      event_map_dealloca(new_map);
	    }
	    
	  }
	  
	}
	
      }

    }

 
  
  }

  /*remake process list to enact object creations & port/instance changes*/
  if ((remake_process_list()) < 0){
    printf("error occured while remaking process list\n");
  }

  /*free all elements in linked list*/
  while (to_load_list)    ll_remove(to_load_list,&to_load_list);

  return 0;
}

xmlNodePtr xml_rtobject_node_get_loaded(xmlNodePtr rtobj_node){
  xmlNodePtr tmp;

  for (tmp=rtobj_node->children;tmp;tmp=tmp->next)
    if ((tmp->name)&&(!strcmp((char*)tmp->name,"Loaded"))) return tmp;

  return 0;
}

rtobject_t* rtobject_load_from_xml(xmlNodePtr rtobj_node, ll_head file_node_list){
  xmlNodePtr tmp;
  xmlNodePtr tmp2;
  char *rdout, *rdout2, *stored_pathname;
  node_t* temp_node;
  int i,j,first_instance;
  rtobject_t* curr_rtobject;
  data_port_t* curr_port;

  /*begin RTObject initiation data*/
  rtobject_t* new_rtobj;
  int new_address;
  int new_major_type;
  int new_imp_type;
  char* new_name;
  char* new_desc;
  int new_argc;
  char** new_argv;
  signal_path_t* new_target_path;
  /*end RTObject initiation data*/

  new_target_path=0;

  /*check Version info, we will only load version 2.XX & 1.XX*/
  if (!(rdout = xmlGetProp(rtobj_node, "Version"))){
    printf("xml error: couldn't read attribute Version\n");
    return 0;
  }
  if ((strlen(rdout) < 2)||\
      ((rdout[0] != '1')&&(rdout[0] != '2'))||\
      (rdout[1] != '.')){
    printf("xml error: can't load rtobject because it's Version %s\n", rdout);
    printf("This copy of Soundtank can only load Versions 2.XX & 1.XX\n");
    return 0;
  }

  if (!(rdout = xmlGetProp(rtobj_node, "Name"))){
    printf("xml error: couldn't read attribute Name\n");
    return 0;
  }

  /*pull out name & target path info from name field*/
  new_name =  pathname_get_name(rdout);
  stored_pathname = pathname_get_path(rdout);

  /*look and see if target path refers to another rtobject in xml file*/
  for (temp_node=file_node_list;temp_node;temp_node=temp_node->next){
    tmp = (xmlNodePtr)temp_node->data;
    
    /*current imp: only look at objects already loaded*/
    if ((tmp2 = xml_rtobject_node_get_loaded(tmp))){

      /*must find loaded rtobject in soundtank*/
      if (!(rdout2 = xmlGetProp(tmp2,"Path"))) break;
      if (!(curr_rtobject = get_rtobject_from_path(rdout2))) break;

      /*now we look at the pathname the rtobject was _saved_ with*/
      /*because the create-rtobject fxn may give a different unique name*/
      if (!(rdout2 = xmlGetProp(tmp,"Name"))) break;

      if ( (!(strcmp(stored_pathname,rdout2))) &&\
	   ((rtobject_get_major_type(curr_rtobject))\
	    == RTOBJECT_MAJOR_TYPE_SIGNAL_PATH)){

	new_target_path = (signal_path_t*)curr_rtobject->imp_struct;
	break;

      }

    }

  }

  /*okay, target path doesn't refer to something else in xml file,*/
  /*just look it up among loaded objects*/
  if (!new_target_path){
    if ((curr_rtobject = get_rtobject_from_path(stored_pathname))&&\
	   (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH\
	    == rtobject_get_major_type(curr_rtobject))){
      	new_target_path = (signal_path_t*)curr_rtobject->imp_struct;
    }
  }

  /*as a last resort, just put the loaded object into the current path*/
  if (!new_target_path) new_target_path = curr_path;


  /*get core character traits of rtobject*/  
  if (!(rdout = xmlGetProp(rtobj_node, "MajorType"))){
    printf("xml error: couldn't read attribute MajorType\n");
    return 0;
  }
  if ((new_major_type = string_to_rtobject_major_type(rdout)) < 0){
    printf("xml error: invalid MajorType attribute, %s\n", rdout);
    return 0;
  }

  if (!(rdout = xmlGetProp(rtobj_node, "ImpType"))){
    printf("xml error: couldn't read attribute ImpType\n");
    return 0;
  }
  if ((new_imp_type = string_to_rtobject_imp_type(rdout)) < 0){
    printf("xml error: invalid ImpType attribute, %s\n", rdout);
    return 0;
  }
  
  /*get description (description is optional)*/
  new_desc = ""; 
  for (tmp=rtobj_node->children;tmp;tmp=tmp->next){
    
    if ((tmp->name)&&(!strcmp((char*)tmp->name,"Description")))
      new_desc = strdup((char*)xmlNodeGetContent(tmp));

  }


  /*count all imp args and make blank argv array to hold them*/
  new_argc = 0;
  for (tmp=rtobj_node->children;tmp;tmp=tmp->next){
    
    if ((tmp->name)&&(!strcmp((char*)tmp->name,"ImpArg"))) ++new_argc;

  }

  if (new_argc)
    new_argv = (char**)malloc(new_argc * sizeof(char*));
  else
    new_argv = 0;


  /*fill new argv array*/
  i = 0;
  for (tmp=rtobj_node->children;tmp;tmp=tmp->next){
    if ((tmp->name)&&(!strcmp((char*)tmp->name,"ImpArg"))){

      new_argv[i++] = strdup((char*)xmlGetProp(tmp,"Value"));

    }
  }
   
  
  /*create rtobject*/
  if ((new_address = create_rtobject(new_major_type, new_imp_type, new_argc, \
				     (const char **)new_argv, new_name, \
				     new_desc, new_target_path)) < 0){
    printf("ERROR occured while trying to create rtobject\n");
    return 0;
  }

  /*free up memory*/
  if (new_name) free(new_name);
  if (stored_pathname) free(stored_pathname);

  if (!(new_rtobj = get_rtobject_from_address(new_address))){
    printf("xml load error: could not find rtobject that was just created, incomplete load\n");    
    return 0;
  }
   
  /*after rtobject is created, update xml node to note this*/
  /*this hack allows us to recreate saved heirarchies of signal paths*/
  if (!(rdout2 = rtobject_get_absolute_pathname(new_rtobj))){

    printf("rtobject save to xml error: memory error\n");

  }else{

    /*we make a special node to note that the object has been loaded...*/
    if (!(tmp = xmlNewChild(rtobj_node,NULL,"Loaded",0))){

      printf("xml error: unable to create node\n");

    }else{  

      /*...and write the absolute path it is loaded at into the node*/
      if (!(xmlSetProp(tmp,"Path",rdout2)))
	printf("xml error: unable to store loaded Path\n");

    }
    
    free(rdout2);

  }


  /*NOTE: event map loading has been moved up to file loading function
    because it has to be done after all of a signal path's member
    objects are loaded*/


  /*get port targets*/
  i = 0;
  for (tmp=rtobj_node->children;tmp;tmp=tmp->next){
    if ((tmp->name)&&(!strcmp((char*)tmp->name,"PortDesc"))){

      if (i < rtobject_get_data_port_list_size(new_rtobj)){
	
	if ((curr_port = rtobject_get_data_port(new_rtobj,i++))){

	  rdout = (char*)xmlGetProp(tmp,"PortTargetIndex");
	  j = atoi(rdout);
	  rdout = (char*)xmlGetProp(tmp,"PortTargetName");
	  
	  if ((data_port_set_target(curr_port,\
				    rdout,
				    j)) < 0){
	    printf("xml error: memory error while loading data port target\n");
	  }

	}

      }else{
	printf("too many data port labels\n");
      }
    }
  }


  /*go through ports and attach them*/
  for (i=0;i<rtobject_get_data_port_list_size(new_rtobj);++i){
    data_port_t* curr_port;
    curr_port = rtobject_get_data_port(new_rtobj,i);

    /*only connect ports that have a non-null target*/
    /*also check for target "*" which means parent channel*/
    if ((data_port_get_target_pathname(curr_port))\
	&&(strcmp("",data_port_get_target_pathname(curr_port)))){

      if (!strcmp("*",data_port_get_target_pathname(curr_port))){

	/*attach to parent channel*/
	if ((connect_data_ports(new_rtobj,i, \
				rtobject_get_parent(new_rtobj),\
				data_port_get_target_port(curr_port))) < 0){
	  printf("error: failed to connect data port %d\n",i);
	}  

      }else{

	/*attach to another object*/
	if (get_rtobject_from_path(data_port_get_target_pathname(curr_port))){
	    if ((connect_data_ports(new_rtobj,i, \
				    get_rtobject_from_path(data_port_get_target_pathname(curr_port)),\
				    data_port_get_target_port(curr_port))) < 0){
	      printf("error: failed to connect data port %d\n",i);
	    }  
	}
	
      }

    }

  }
  
  /*restore instance count and all instance control values*/
  first_instance = 1;
  for (tmp=rtobj_node->children;tmp;tmp=tmp->next){
    if ((tmp->name)&&(!strcmp((char*)tmp->name,"RTInstance"))){
      
      if (!first_instance){
	
	if ((create_rtobject_instance(new_rtobj)) < 0){
	  printf("xml load error: couldn't create all instances\n");
	  
	  return 0;
	}
 
	if (!(temp_node = temp_node->next)){
	  printf("xml load error: made instance but rtobject not updated\n");
	  
	  return 0;
	}
	
      }else{
	
	if (!(temp_node = new_rtobj->instance_list)){
	  printf("xml load error: no instance was created\n");
	  
	  return 0;
	}
	  
	first_instance = 0;
      }
      
      i=0;
      for (tmp2=tmp->children;tmp2;tmp2=tmp2->next){
	if ((tmp2->name)&&(!strcmp((char*)tmp2->name,"ControlValue"))){
	
	  if (i < rtobject_get_control_list_size(new_rtobj)){
	    rdout = (char*)xmlGetProp(tmp2,"Value");
	    ((rtobject_instance_t*)temp_node->data)->control_list[i++] = atof(rdout);
	  }else{
	    printf("too many saved control values\n");
	  }

	}
	
      }
      
    }
  }

  return new_rtobj;
}

int rtobject_save_to_file(rtobject_t* rtobj, const char *file_name){
  int i;
  xmlNsPtr soundtankns;
  xmlDocPtr doc;
  xmlNodePtr curr;
  xmlAttrPtr attp;

  /*make a blank XML document*/
  if (!(doc = xmlNewDoc("1.0"))){
    fprintf(stderr,"Document not created successfully. \n");
    return -1;
  }

  /*make a root node that is a SoundtankCollection*/
  if (debug_readout) printf("adding root node\n");
  if (!(curr = xmlNewDocNode(doc,NULL,"SoundtankCollection","\n\n"))){
    printf("xml error: unable to create root node\n");
    xmlFreeDoc(doc);  
    return -1;
  }

  if (debug_readout) printf("setting saved file internal version: 1.0\n");
  if (!(attp = xmlSetProp(curr,"Version","1.0"))){
    printf("xml error: unable to set saved file internal version\n");
    xmlFreeDoc(doc);  
    return -1;
  }

  if (xmlDocSetRootElement(doc,curr)){
    if (debug_readout) printf("replaced root element\n");
  }else
    if (debug_readout) printf("set root element\n");


  if (debug_readout) printf("attaching to namespace\n");
  if (!(soundtankns = xmlNewNs(curr,"http://soundtank.sourceforge.net/xml/soundtank_rtobject_xml_v1.0.dtd","sndtnk"))){
    printf("xml error: unable to find soundtank Document Type Definition\n");
    xmlFreeDoc(doc);  
    return -1;
  }

  if ((rtobject_save_to_xml(rtobj, curr)) < 0){
    printf("xml error: failed attempt to save rtobject to xml\n");
    xmlFreeDoc(doc);  
    return -1;
  }

  /*TODO: pretty formatting of xml doc for human readability*/
   
  /*write XML document to file system*/
  printf("saving as %s    ", file_name);
  i = xmlSaveFormatFile(file_name, doc, 1);   
  printf("wrote %d bytes\n", i);

  /*release resources*/
  xmlFreeDoc(doc); 

  return 0;
}

int rtobject_save_to_xml(rtobject_t* rtobj, xmlNodePtr rtobj_node){
  xmlNodePtr curr;
  xmlAttrPtr attp; 
  node_t* temp_node;
  int i, skip;
  char pass[64];
  char* indent1 = "\n  ";
  char* indent2 = "\n    ";
  char *abs_pathname, *ret;

  /*screening: don't save master path, or the top-level paths in, src,
    efx, out*/
  skip = 0;
  /*TODO: replace this by making fxn 'rtobject_is_master_path', no one
    should refer to the master path like this*/
  if (rtobject_get_parent(rtobj) == rtobj) skip = 1;
  if (rtobject_get_parent(rtobj) == master_path_rtobject){
    if (!strcmp(rtobject_get_name(rtobj),"in")) skip = 1;
    if (!strcmp(rtobject_get_name(rtobj),"src")) skip = 1;
    if (!strcmp(rtobject_get_name(rtobj),"efx")) skip = 1;
    if (!strcmp(rtobject_get_name(rtobj),"out")) skip = 1;    
  }

  if (!skip){

  /*a little space before rtobject starts*/
  xmlAddChild(rtobj_node,xmlNewText("\n\n"));

  /*make a child node that is an RTObject*/
  printf("making node for RTObject: %s\n",rtobject_get_name(rtobj));
  if (!(curr = xmlNewChild(rtobj_node,NULL,"RTObject","\n"))){
    printf("xml error: unable to create node\n");
      
    return -1;
  }

  /*save Soundtank xml dtd version on per-rtobject basis because a
    Soundtank collection will be able to contain rtobjects of
    different versions in the future*/
  if (!(attp = xmlNewProp(curr,"Version","2.0"))){
    printf("xml error: unable to set attribute: Version\n");
    return -1;
  }

  /*store absolute pathname (must use absolute pathname to recreate
    path heirarchy when loading)*/
  if (!(abs_pathname = rtobject_get_absolute_pathname(rtobj))){
    printf("rtobject save to xml error: memory error\n");
    return -1;
  }
  if (!(attp = xmlNewProp(curr,"Name",abs_pathname))){
    printf("xml error: unable to set attribute: Name\n");
    return -1;
  }
  free(abs_pathname);

  /*store major type*/
  if (!(ret = rtobject_major_type_to_string(rtobject_get_major_type(rtobj)))){
    printf("xml error: couldn't get major type of rtobject\n");
    return -1;
  }
  if (!(attp = xmlNewProp(curr, "MajorType", ret))){
    printf("xml error: unable to set attribute: MajorType\n");   
    return -1;
  }
  free(ret);

  /*store implementation type*/
  if (!(ret = rtobject_imp_type_to_string(rtobject_get_implementation_type(rtobj)))){
    printf("xml error: couldn't get imp type of rtobject\n");
    return -1;
  }
  if (!(attp = xmlNewProp(curr, "ImpType", ret))){
    printf("xml error: unable to set attribute: ImpType\n");
    return -1;
  }
  free(ret);

  /*make a child node for the description*/
  if (strlen(rtobject_get_description(rtobj))){

    if (debug_readout) printf("writing rtobject description\n");
    if (!(curr = xmlNewChild(curr,NULL,"Description",rtobject_get_description(rtobj)))){
      printf("xml error: unable to create node Description\n");
        
      return -1;
    }

    curr = curr->parent;

  }

  /*make sibling nodes for the implementation arguments*/
  if (debug_readout) printf("there are %d imp args to save\n",rtobject_get_implementation_arg_list_size(rtobj));
  for (i=0;i<rtobject_get_implementation_arg_list_size(rtobj);++i){

    xmlAddChild(curr,xmlNewText(indent1));

    if (debug_readout) printf("writing an implementation argument\n");
    if (!(curr = xmlNewChild(curr,NULL,"ImpArg",0))){
      printf("xml error: unable to create node ImpArg\n");
        
      return -1;
    }

    if (debug_readout) printf("storing value of implementation argument %d\n",i);
    if (!(attp = xmlNewProp(curr,"Value",rtobj->imp_arg_list[i]))){
      printf("xml error: unable to set attribute: Arg Value\n");
        
      return -1;
    }

    curr = curr->parent;

  }

 
  /*save port labels*/
  if (debug_readout) printf("there are %d ports to save\n",\
			    rtobject_get_data_port_list_size(rtobj));

  /*NOTE: don't save port labels for signal paths because those ports
  don't actually belong to the path, let them be saved with the local
  out's & in's*/
  if (rtobject_get_major_type(rtobj) != RTOBJECT_MAJOR_TYPE_SIGNAL_PATH){
    for (i=0;i<rtobject_get_data_port_list_size(rtobj);++i){
      data_port_t* curr_port;
      channel_t* curr_port_chan;
      curr_port = rtobject_get_data_port(rtobj,i);
      
      if (debug_readout) printf("writing a port description\n");
      xmlAddChild(curr,xmlNewText(indent1));
      if (!(curr = xmlNewChild(curr,NULL,"PortDesc",0))){
	printf("xml error: unable to create node PortDesc\n");        
	return -1;
      }
      
      /*check if port is attached to anything*/
      if ((curr_port_chan = data_port_get_channel(curr_port))){

	/*check if port is attached to local channel*/
	if (channel_get_scope(curr_port_chan) == CHANNEL_SCOPE_LOCAL){

	  /*store port target name to parent*/
	  if (debug_readout) printf("writing a port target name\n");
	  if (!(attp = xmlNewProp(curr, "PortTargetName", "*"))){
	    printf("xml error: unable to set attribute: port target name\n");
	    return -1;
	  }
      
	  /*store port's target port index to local channel index*/
	  snprintf(pass,64,"%d",channel_get_index(curr_port_chan));
     
	  if (debug_readout) printf("storing index of target port %d\n",i);
	  if (!(attp = xmlNewProp(curr,"PortTargetIndex",pass))){
	    printf("xml error: unable to set attribute: port target index\n");   
	    return -1;
	  }      	  

	}else{
	  /*non-local attachment, must store target name & port*/

	  /*store port target name*/
	  if (debug_readout) printf("writing a port target name\n");
	  if (!(attp = xmlNewProp(curr,"PortTargetName",\
				data_port_get_target_pathname(curr_port)))){
	    printf("xml error: unable to set attribute: port target name\n");
	    return -1;
	  }
      
	  /*store port's target port index*/
	  snprintf(pass,64,"%d",data_port_get_target_port(curr_port));
	
	  if (debug_readout) printf("storing index of target port %d\n",i);
	  if (!(attp = xmlNewProp(curr,"PortTargetIndex",pass))){
	    printf("xml error: unable to set attribute: port target index\n");   
	    return -1;
	  }      
	  
	}

      }else{
	/*port isn't attached, make empty target*/
	if (debug_readout) printf("writing a port target name\n");
	if (!(attp = xmlNewProp(curr,"PortTargetName",""))){
	  printf("xml error: unable to set attribute: port target name\n");    
	  return -1;
	}
      
	/*make empty target port index*/	
	if (debug_readout) printf("storing index of target port %d\n",i);
	if (!(attp = xmlNewProp(curr,"PortTargetIndex",""))){
	  printf("xml error: unable to set attribute: port target index\n");   
	  return -1;
	}      
	
      }

      curr = curr->parent;

    }

  }
  
  /*save all instances' control settings*/
  for (temp_node=rtobj->instance_list;temp_node;temp_node=temp_node->next){

    if (debug_readout) printf("making an RTInstance node\n");
    xmlAddChild(curr,xmlNewText(indent1));
    if (!(curr = xmlNewChild(curr,NULL,"RTInstance",0))){
      printf("xml error: unable to create node RTInstance\n");
        
      return -1;
    }

    /*save all control values*/
    for (i=0;i<rtobject_get_control_list_size(rtobj);++i){

     xmlAddChild(curr,xmlNewText(indent2));
     if (!(curr = xmlNewChild(curr,NULL,"ControlValue",0))){
	printf("xml error: unable to create node ControlValue\n");
	  
	return -1;
      }

      snprintf(pass,64,"%f",((rtobject_instance_t*)temp_node->data)->control_list[i]);
      if (debug_readout) printf("storing value of control %d\n",i);
      if (!(attp = xmlNewProp(curr,"Value",pass))){
	printf("xml error: unable to set attribute: Value\n");
	  
	return -1;
      }
      
      curr = curr->parent;

    }

    curr = curr->parent;

  }

  /*save all event maps*/
  for (i=0;i<rtobject_get_map_list_size(rtobj);++i){
    event_map_t* map;
    
    if (rtobject_get_map(rtobj, &map, i) < 0){
      printf("xml error: unable to get event map %d\n", i);
      return -1;
    }

    if (event_map_save_to_xml(map, &curr, rtobj) < 0){
      printf("xml error: unable to save event map %d\n",i);
      return -1;
    }

  }

  xmlAddChild(curr->parent,xmlNewText("\n\n"));

  }

  /*if this is a signal path, recurse to save all children*/
  if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(rtobj)){
    rtobject_t* curr_obj;
    signal_path_t* sigpath;
    
    sigpath = (signal_path_t*)rtobj->imp_struct;

    for (temp_node = sigpath->object_list;temp_node;temp_node=temp_node->next){
      curr_obj = (rtobject_t*)temp_node->data;
      
      /*save rtobject*/
      if (debug_readout) printf("saving RTObject %s\n",rtobject_get_name(curr_obj));
      if ((rtobject_save_to_xml(curr_obj, rtobj_node)) < 0){
	printf("xml error: unable to save RTObject\n");
	return -1;
      }
    
    }

  }

  return 0;
}

