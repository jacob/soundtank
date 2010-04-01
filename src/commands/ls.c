/*
 * soundtank internal commands code: list
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



/********* LOCAL ls CONTROL VARIABLES ***************/
int long_rdout, show_ports, show_controls, show_instances, show_full, show_channels, suppress_headers;



void print_column(const char *data, int width){

  if ( (strlen(data)) > width){

    char *temp = strdup(data);
    temp[width] = '\0';
    printf("%s",temp);
    free(temp);

  }else{

    int i;
    printf("%s",data);
    for (i=0;i<(width - (strlen(data)));++i) printf(" ");

  }

}

void rtobject_printout_info(rtobject_t* rtobj){
  node_t* temp_node;
  int i,j;
  data_port_t* curr_port;
  channel_t* curr_chan;
  char temp[128], *ret;

  if (!long_rdout){

    printf("%s    ",rtobject_get_name(rtobj));

    if ((show_ports)||(show_controls)||(show_instances)||(show_channels)) printf("\n");

  }else{

    /*printout major type*/
    ret = rtobject_major_type_to_string(rtobject_get_major_type(rtobj));
    if (ret){
      printf("%s", ret);
      free(ret);
    }
    
    /*printout name*/
    printf(" %s    ",rtobject_get_name(rtobj));

    /*printout address*/
    if (!suppress_headers) printf("address: ");
    printf("%d  ",rtobject_get_address(rtobj));

    /*print out imp typ*/
    if (!suppress_headers)    printf("imp type: ");
    ret = rtobject_imp_type_to_string(rtobject_get_implementation_type(rtobj));
    if (ret){
      printf("%s", ret);
      free(ret);
    }

    /*print out any imp args*/
    for (i=0;i<rtobject_get_implementation_arg_list_size(rtobj);++i){
      if (!i) printf(" (");
      printf("%s ",rtobject_get_implementation_arg(rtobj,i));
    }
    if (i) printf(") ");

    /*special handling for LADPSA Plugins*/
    if (rtobject_get_implementation_type(rtobj)\
	== RTOBJECT_IMP_TYPE_LADSPA_PLUGIN){

      printf("\n  ");
      print_imp_object_ladspa_plugin(rtobj);
      printf("\n");

    }

    /*printout parent path*/
    if (rtobject_get_parent_path(rtobj)){    
      
      if (!suppress_headers) 
	printf("  in path:");
      else 
	printf(" in: ");

      printf("%s  ",rtobject_get_name(get_rtobject_from_address(signal_path_get_owner_rtobject_address(rtobject_get_parent_path(rtobj)))));
 
    }else{
      printf("  not in any path  ");
    }

    if (!suppress_headers) 
      printf("process index:");
    printf("%d  \n",rtobject_get_process_index(rtobj));
    
  }
   
  /*print out data ports*/
  if (show_ports){

    if (!suppress_headers) 
      printf("%d data port(s):\n",rtobject_get_data_port_list_size(rtobj));
    
    for (i=0;i<(rtobject_get_data_port_list_size(rtobj));++i){
      if (!(curr_port = rtobject_get_data_port(rtobj,i))){
	printf("error can't access port %d\n",i);
	
      }
      
      if (data_port_get_input(curr_port))
	printf(" input ");
      else
	printf(" output ");

      printf("data port %d, ",i);

      if (data_port_get_description_string(curr_port))
	printf("\"%s\" ",data_port_get_description_string(curr_port));
      
      if ((curr_chan = data_port_get_channel(curr_port))){

	printf("is connected to ");

	if (!(data_port_get_input(curr_port))){
	  if (!(strcmp(data_port_get_target_pathname(curr_port),"*"))){
	    printf("parent ");
	  }else{
	    printf("%s, port %d via ",\
		   data_port_get_target_pathname(curr_port),\
		   data_port_get_target_port(curr_port));
	  }
	}

	if (channel_get_scope(curr_chan) == CHANNEL_SCOPE_GLOBAL)
	  printf("global ");
	else if (channel_get_scope(curr_chan) == CHANNEL_SCOPE_LOCAL)
	  printf("local ");
	else
	  printf("SHARED????");
	printf("channel %d\n",channel_get_index(curr_chan));

      }else{

	printf("isn't connected\n");

      }
    }
  }    

  /*print out controls*/
  if (show_controls){

    if (!suppress_headers) 
      printf("%d control(s):\n",rtobject_get_control_list_size(rtobj));
    
    for (i=0;i<rtobject_get_control_list_size(rtobj);++i){
      const control_t* curr_control = rtobject_get_control(rtobj, i);

      if (!curr_control){

	printf("error can't access control %d\n",i);
	
      }else{
	
	/*printf control index*/
	printf(" # %2d, ",i);

	/*print control description*/
	snprintf(temp,128,"\"%s\"   ", control_get_desc_string(curr_control));
	print_column(temp,18);

	/*print out all the instance values for this control */
	for (temp_node=rtobj->instance_list;temp_node;temp_node=temp_node->next){

	  snprintf(temp,128,"%.3f",((rtobject_instance_t*)temp_node->data)->control_list[i]);
	  print_column(temp,8);
	  /*ensure a separation between numbers*/
	  printf(" ");

	}

	printf("\n");

      }

    }

  }
    

  /* print out instances*/
  if (show_instances){

    if (!suppress_headers) 
      printf("%d instance(s):\n",rtobj->instance_list_size);
    
    /*print out control names as a head line to make sense of values */
    printf("              ");
    for (i=0;i<rtobject_get_control_list_size(rtobj);++i){
      const control_t* curr_control = rtobject_get_control(rtobj, i);

      if (!curr_control){

	printf("error can't access control %d\n",i);
      
      }else{
	char temp[128];
	snprintf(temp,128,"%2d: %s", i, control_get_desc_string(curr_control));
	print_column(temp,15);
      }

    }
    printf("\n");

    i=0;
    for (temp_node=rtobj->instance_list;temp_node;temp_node=temp_node->next){
      printf(" instance %2d   ",i++);
      
      /*print instance control values*/
      for (j=0;j<rtobject_get_control_list_size(rtobj);++j){
	char temp[128];
	snprintf(temp,128,"%d: %.3f",j,((rtobject_instance_t*)temp_node->data)->control_list[j]);
	print_column(temp,15);
	
      }
      
      printf("\n");
      
    }
    
  }

  /* print out signal path channels*/
  if ((show_channels)&&(RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(rtobj))){
    signal_path_t* sigpath;
    channel_t* curr_chan;

    sigpath = (signal_path_t*)rtobj->imp_struct;
    
    if (!suppress_headers) 
      printf("signal path channels: \n");

    i=0;
    for (temp_node=sigpath->channel_list;temp_node;temp_node=temp_node->next){
      curr_chan = (channel_t*)temp_node->data;

      if ((CHANNEL_SCOPE_LOCAL == channel_get_scope(curr_chan))||\
	  (CHANNEL_SCOPE_GLOBAL == channel_get_scope(curr_chan))){
	printf(" channel %d, index %d   ",i++,channel_get_index(curr_chan));
      }

    }

  }

 
 
}






void soundtank_command_ls(int argc, char** argv){
  int i,j;
  rtobject_t* curr_obj;
  node_t* temp_node;
  struct poptOption optarray[18];
  poptContext optcontext;
  const char ** ret_imp_argv; /*for extra arguments not handled by popt*/
  const char* curr_arg;
  int ret_imp_argc;

  int disp_buffers, disp_plugins, disp_instances, disp_address, disp_help;
  int search_id, search_apropos;
  int opt_recurse;
  disp_buffers = disp_plugins = disp_instances = disp_address = disp_help = search_id = search_apropos = opt_recurse = 0;


  /*initialize module scope control variables*/
  long_rdout = suppress_headers = 0;
  show_ports = show_controls = show_instances = show_full = show_channels = 0;

  /*Parse Options using library popt*/

  /*fill poptOption array, this handles option parseing*/
  if (!(optarray[0].longName = strdup("long"))) {return ;}
  optarray[0].shortName = 'l';
  optarray[0].argInfo = POPT_ARG_NONE;
  optarray[0].arg = (void*)&long_rdout; 
  optarray[0].val = 1;
  optarray[0].descrip ="long format readout";
  optarray[0].argDescrip = "boolean";

  if (!(optarray[1].longName = strdup("buffers"))) {return ;}
  optarray[1].shortName = 'b';
  optarray[1].argInfo = POPT_ARG_NONE;
  optarray[1].arg = (void*)&disp_buffers; 
  optarray[1].val = 2;
  optarray[1].descrip = "display buffers";
  optarray[1].argDescrip = "boolean";

  if (!(optarray[2].longName = strdup("plugins"))) {return ;}
  optarray[2].shortName = 'q';
  optarray[2].argInfo = POPT_ARG_NONE;
  optarray[2].arg = (void*)&disp_plugins; 
  optarray[2].val = 3;
  optarray[2].descrip = "display ladspa plugins";
  optarray[2].argDescrip = "boolean";

  if (!(optarray[3].longName = strdup("id"))) {return ;}
  optarray[3].shortName = 'z';
  optarray[3].argInfo = POPT_ARG_NONE;
  optarray[3].arg = (void*)&search_id; 
  optarray[3].val = 4;
  optarray[3].descrip = "search by ladspa id";
  optarray[3].argDescrip = "boolean";

  if (!(optarray[4].longName = strdup("apropos"))) {return ;}
  optarray[4].shortName = 'k';
  optarray[4].argInfo = POPT_ARG_NONE;
  optarray[4].arg = (void*)&search_apropos; 
  optarray[4].val = 5;
  optarray[4].descrip = "search for keyword";
  optarray[4].argDescrip = "boolean";

  if (!(optarray[5].longName = strdup("proclist"))) {return ;}
  optarray[5].shortName = 'u';
  optarray[5].argInfo = POPT_ARG_NONE;
  optarray[5].arg = (void*)&disp_instances; 
  optarray[5].val = 6;
  optarray[5].descrip = "display instances";
  optarray[5].argDescrip = "boolean";

  if (!(optarray[6].longName = strdup("address"))) {return ;}
  optarray[6].shortName = 'a';
  optarray[6].argInfo = POPT_ARG_NONE;
  optarray[6].arg = (void*)&disp_address; 
  optarray[6].val = 6;
  optarray[6].descrip = "display all occupied addresses";
  optarray[6].argDescrip = "boolean";

  if (!(optarray[7].longName = strdup("recurse"))) {return ;}
  optarray[7].shortName = 'r';
  optarray[7].argInfo = POPT_ARG_NONE;
  optarray[7].arg = (void*)&opt_recurse; 
  optarray[7].val = 7;
  optarray[7].descrip = "list recursively";
  optarray[7].argDescrip = "boolean";

  if (!(optarray[8].longName = strdup("ports"))) {return ;}
  optarray[8].shortName = 'p';
  optarray[8].argInfo = POPT_ARG_NONE;
  optarray[8].arg = (void*)&show_ports; 
  optarray[8].val = 7;
  optarray[8].descrip = "show ports";
  optarray[8].argDescrip = "boolean";

  if (!(optarray[9].longName = strdup("controls"))) {return ;}
  optarray[9].shortName = 'c';
  optarray[9].argInfo = POPT_ARG_NONE;
  optarray[9].arg = (void*)&show_controls; 
  optarray[9].val = 7;
  optarray[9].descrip = "show controls";
  optarray[9].argDescrip = "boolean";

  if (!(optarray[10].longName = strdup("instances"))) {return ;}
  optarray[10].shortName = 'i';
  optarray[10].argInfo = POPT_ARG_NONE;
  optarray[10].arg = (void*)&show_instances; 
  optarray[10].val = 10;
  optarray[10].descrip = "show instances";
  optarray[10].argDescrip = "boolean";

  if (!(optarray[11].longName = strdup("channels"))) {return ;}
  optarray[11].shortName = 'h';
  optarray[11].argInfo = POPT_ARG_NONE;
  optarray[11].arg = (void*)&show_channels; 
  optarray[11].val = 11;
  optarray[11].descrip = "show channels";
  optarray[11].argDescrip = "boolean";

  if (!(optarray[12].longName = strdup("full"))) {return ;}
  optarray[12].shortName = 'f';
  optarray[12].argInfo = POPT_ARG_NONE;
  optarray[12].arg = (void*)&show_full; 
  optarray[12].val = 12;
  optarray[12].descrip = "show full detail";
  optarray[12].argDescrip = "boolean";

  if (!(optarray[13].longName = strdup("suppress"))) {return ;}
  optarray[13].shortName = 's';
  optarray[13].argInfo = POPT_ARG_NONE;
  optarray[13].arg = (void*)&suppress_headers; 
  optarray[13].val = 13;
  optarray[13].descrip = "suppress headers";
  optarray[13].argDescrip = "boolean";

  if (!(optarray[14].longName = strdup("help"))) {return ;}
  optarray[14].shortName = '?';
  optarray[14].argInfo = POPT_ARG_NONE;
  optarray[14].arg = (void*)&disp_help;
  optarray[14].val = 0;
  optarray[14].descrip = "Display Help Message";
  optarray[14].argDescrip = 0;

  optarray[15].longName =0;
  optarray[15].shortName = '\0';
  optarray[15].argInfo = 0;
  optarray[15].arg = 0; 
  optarray[15].val = 0;


  /*parse poptOption array*/
  optcontext = poptGetContext("list_flags",argc,(const char**)argv,optarray,0);
  j=0;
  while ((i=poptGetNextOpt(optcontext)) > 0){
    ++j;
    if ((i<0)&&(i!=-1)){
      printf("\n invalid option %s\n",argv[(j*2)+1]); /*TODO doesn't work*/
      return;
    }
  }

  /*handle the remaining arguments */
  ret_imp_argv = poptGetArgs(optcontext);
  ret_imp_argc = 0;

  if (ret_imp_argv){
    /*count how many non-option args*/
    for (i=0;;++i){
      curr_arg = ret_imp_argv[i];
      if (!curr_arg) break;
      if (debug_readout) printf("arg %d=%s.\n",i,curr_arg);
    }
    ret_imp_argc = i;
  }

  /*handle some arguments*/
  if (show_full) show_ports = show_controls = show_instances = show_channels = long_rdout = 1;

  /*check for contradicting arguments (don't worry about help, it overrides all)*/
  {
    int err_flag = 0;
    if (disp_plugins)
      if ((disp_buffers)||(disp_instances)) err_flag=1;
    if (disp_buffers)
      if ((disp_plugins)||(disp_instances)) err_flag=1;
    if (disp_instances)
      if ((disp_plugins)||(disp_buffers)) err_flag=1;
    if (disp_address)
      if ((disp_plugins)||(disp_buffers)||(disp_instances)) err_flag=1;
    

    if (err_flag){
      printf("list error: options conflict, two different types of listing requested\n");
      return;
    }
  }

  /*Handle Cases*/

  if (disp_help){
    printf("Usage: ls [OPTION] [b|q|u|a] [object-address|object-name] \n");
    printf("Explanation: ls lists the realtime objects currently loaded in Soundtank\n");
    printf("   -l, --long         long format readout \n");
    printf("   -p, --ports        show data ports\n");
    printf("   -c, --controls     show controls\n");
    printf("   -i, --instances    show instances\n");
    printf("   -h, --channels     show channels for signal path objects\n");
    printf("   -b, --buffers      display Soundtank internal buffers instead of rtobjects\n");
    printf("   -q, --plugins      display available LADSPA plugins instead of rtobjects\n");
    printf("   -z, --id           search available LADSPA plugins by id number\n");
    printf("   -k, --apropos      search available LADSPA plugins by keyword\n");
    printf("   -u, --proclist     display only instances instead of rtobjects\n");
    printf("   -a, --address      display all occupied addresses\n");
    printf("   -r, --recursive    list signal path contents recursively\n");
    printf("   -s, --suppress     suppress headers\n");
    printf("   -?, --help         display this help message\n");
    return;
  }


  if (disp_buffers){
    int count;

    if (ret_imp_argc > 0)
      count = atoi(ret_imp_argv[0]);
    else 
      count = 0;
    
    printf("\n%d buffers:\n\n",ll_get_size(&buffer_list));
    for (temp_node=buffer_list;temp_node;temp_node=temp_node->next){
      buffer_debug_print(((buffer_t*)temp_node->data), count);
      printf("\n");
    }

  }else if (disp_plugins){

    if (search_id){

      if (!ret_imp_argc){
	printf("not enough arguments\n");
	return;
      }
      set_ladspa_search_id((unsigned long)atol(ret_imp_argv[0]));
      LADSPAPluginSearch(idsearchPluginLibrary);	

    }else if (search_apropos){

      if (!ret_imp_argc){
	printf("not enough arguments\n");
	return;
      }
      set_ladspa_search_label(ret_imp_argv[0]);
      LADSPAPluginSearch(approposearchPluginLibrary);	
      set_ladspa_search_label(0);	

    }else{

      LADSPAPluginSearch(describePluginLibrary);

    }

  }else if (disp_instances){

    printf("Process List:  (%d instances)\n",process_list_size);
    for (i=0;i<process_list_size;++i)
      printf(" instance %d: address %p \n", i, process_list[i]);

  }else if (disp_address){
    char *ret;

    printf("Master Address List\n");
    for(i=0;i<rtobject_address_list_size;++i){
      curr_obj = rtobject_address_list[i];
      if (curr_obj != 0){
	printf("  Address:%d  ",rtobject_get_address(curr_obj));
	ret = rtobject_major_type_to_string(rtobject_get_major_type(curr_obj));
	if (ret){
	  printf("%s", ret);
	  free(ret);
	}
	printf("  %s   -->",rtobject_get_name(curr_obj));
	printf("  %s\n",rtobject_get_name(rtobject_get_parent(curr_obj)));
      }      
    }

  }else{
    /*just print out the specified object*/

    if (ret_imp_argc){
      if (!(curr_obj = get_rtobject_from_path(ret_imp_argv[0]))){
	printf("could not find rtobject %s\n",ret_imp_argv[0]);
	return ;
      }
    }else{
      curr_obj = get_rtobject_from_address(signal_path_get_owner_rtobject_address(curr_path));
    }

    
    if (RTOBJECT_MAJOR_TYPE_SIGNAL_PATH == rtobject_get_major_type(curr_obj)){
      /*print out member objects of signal paths*/
      node_t* temp_node;
      signal_path_t* sigpath;
      int printed_last_return;
      sigpath = (signal_path_t*)curr_obj->imp_struct;

      printed_last_return = 0;

      for (temp_node=sigpath->object_list;temp_node;temp_node=temp_node->next){
	rtobject_printout_info((rtobject_t*)temp_node->data);
	if ((show_ports)||(show_controls)||(show_instances)||(show_channels)){
	  printf("\n");
	  printed_last_return = 1;
	}
      }  

      if ((!printed_last_return)&&(sigpath->object_list))
	  printf("\n");

    }else{

      rtobject_printout_info(curr_obj);
      printf("\n");

    }

  
  }
   
}

