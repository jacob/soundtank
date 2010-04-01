/*
 * ladspa plugin wrapper rtobject code
 *
 * Copyright Jacob Robbins 2003-2004
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

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <ladspa.h>

#include "../include.h"
#include "../soundtank_structs.h"


unsigned long load_id;
const char* load_label;
ladspa_plugin_t* load_imp_object;




ladspa_plugin_t*  imp_object_ladspa_plugin_alloca(){
  ladspa_plugin_t* new_plugin;

  if (!(new_plugin = (ladspa_plugin_t*)malloc(sizeof(ladspa_plugin_t)))){
    printf("ladspa plugin alloca: memory error\n");
    return 0;
  }

  new_plugin->LADSPA_Plugin_Library=0;
  new_plugin->LADSPA_Descriptor=0;
  new_plugin->use_run_adding_emulation=0;

  return new_plugin;
}


void imp_object_ladspa_plugin_dealloca(ladspa_plugin_t* oldobj){

  unloadLADSPAPluginLibrary(oldobj->LADSPA_Plugin_Library);
  free(oldobj);

}



int create_imp_object_ladspa_plugin(rtobject_t* rtobj){
  ladspa_plugin_t* new_plugin;
  int output_port_count, i;
  data_port_t* curr_port;
  control_t* curr_control;

  /*sanity check: need at least a plugin label*/
  if (rtobject_get_implementation_arg_list_size(rtobj) < 1){
    printf("create ladspa plugin imp object error: not enought args, need a plugin name\n");
    return -1;
  }

  /*allocate imp struct*/
  if (!(new_plugin = (void*)imp_object_ladspa_plugin_alloca())){
    printf("create ladspa plugin imp error: failed attempt to allocated imp object\n");
    return -1;
  }

  /*check for overwrite mode*/
  new_plugin->use_run_adding_emulation = 1;
  for (i = 0; i < rtobject_get_implementation_arg_list_size(rtobj); ++i){
    if (!(strcmp(rtobject_get_implementation_arg(rtobj, i), "-o")))
      new_plugin->use_run_adding_emulation = 0;
  }

  /*check if plugin id is given*/
  if ((!(strcmp(rtobject_get_implementation_arg(rtobj, 0),"-i")))||\
      (!(strcmp(rtobject_get_implementation_arg(rtobj, 0),"--id")))){

    /*sanity check: need to give an id in this case*/   
    if (rtobject_get_implementation_arg_list_size(rtobj) < 2){
      printf("create ladspa plugin imp object error: not enought args, need a plugin id\n");
      return -1;
    }

    load_id = atol(rtobject_get_implementation_arg(rtobj, 1));
    load_label = 0;

  }else{
    /*no id given, assume first arg is a plugin label*/
    load_id = 0;
    load_label = rtobject_get_implementation_arg(rtobj, 0);
    
  }

  load_imp_object = new_plugin;


  /*try to load plugin*/
  LADSPAPluginSearch(ladspa_plugin_loading_search);	

  /*see if plugin was loaded and attach to rtobject*/
  if (!new_plugin->LADSPA_Descriptor){
    printf("create ladspa plugin imp error: couldn't find plugin requested\n");
    printf("You probably haven't installed a necessary LADSPA Plugin \n");
    free(new_plugin); /*can't unload library cause it's not loaded*/
    return -1;
  }
  rtobj->imp_struct = (void*)new_plugin;
 
  /*currently can not handle inplace-broken plugins*/
  if (LADSPA_IS_INPLACE_BROKEN(new_plugin->LADSPA_Descriptor->Properties)){
    printf("create ladspa plugin imp error: plugin is inplace broken and so can't be used\n");
    imp_object_ladspa_plugin_dealloca(new_plugin);
    rtobj->imp_struct = 0;
    return -1;
  }

  /*make 3 controls: active, mute and volume*/
 
  /*control 0: active*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_ACTIVE))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }
    
  /*control 1: mute*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_MUTE))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }
    
  /*control 2: volume*/
  if (!(curr_control = control_create_from_desc_code(CONTROL_DESC_VOLUME))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }  
  if (!(ll_append(&rtobj->control_list,(void*)curr_control))){
    printf("create imp object alsa out error: memory error\n");
    return -1;
  }


  /*make controls & data ports for LADSPA plugin's ports*/
  for (i=0;i<new_plugin->LADSPA_Descriptor->PortCount;++i){

    if LADSPA_IS_PORT_AUDIO(new_plugin->LADSPA_Descriptor->PortDescriptors[i]){

      if (!(curr_port = \
	    data_port_create_from_ladspa_port(&new_plugin->LADSPA_Descriptor->PortDescriptors[i],
					      new_plugin->LADSPA_Descriptor->PortNames[i],
					      &new_plugin->LADSPA_Descriptor->PortRangeHints[i],
					      i)) < 0){
	printf("LADSPA error: failed attempt to initialize LADSPA port %d\n", i);
	return -1;
      }

      if (!(ll_append(&rtobj->data_port_list, (void*)curr_port))){
	printf("create imp object alsa out error: memory error\n");
	return -1;
      }
	
    }

    if LADSPA_IS_PORT_CONTROL(new_plugin->LADSPA_Descriptor->PortDescriptors[i]){

      if (!(curr_control = \
	    control_create_from_ladspa_port(&new_plugin->LADSPA_Descriptor->PortDescriptors[i],
					    new_plugin->LADSPA_Descriptor->PortNames[i],
					    &new_plugin->LADSPA_Descriptor->PortRangeHints[i],
					    i)) < 0){

	printf("LADSPA error: failed attempt to initialize LADSPA port %d\n", i);
	return -1;
      }

      if (!(ll_append(&rtobj->control_list, (void*)curr_control))){
	printf("create imp object alsa out error: memory error\n");
	return -1;
      }
      
    }

  }
  
  /*record data port count*/
  rtobj->data_port_list_size = ll_get_size(&rtobj->data_port_list);

  /*attach output data ports to working buffers if necessary*/
  if (new_plugin->use_run_adding_emulation){
    buffer_t* curr_working_buff;
    data_port_t* curr_port;

    output_port_count=0;
    for (i=0;i<rtobj->data_port_list_size;++i){
      curr_port = rtobject_get_data_port(rtobj,i);

      if (!(data_port_get_input(curr_port))){
	
	if (!(curr_working_buff = get_working_buffer(output_port_count++))){
	  printf("create LADSPA plugin imp error: failed attempt to get a working buffer\n");
	  return -1;
	}

	curr_port->working_buffer = curr_working_buff;

      }
    
    }

  }

  return 0;
}

int destroy_imp_object_ladspa_plugin(rtobject_t* rtobj){
  
  imp_object_ladspa_plugin_dealloca((ladspa_plugin_t*)rtobj->imp_struct);
  rtobj->imp_struct=0;

  return 0;
}


int init_instance_ladspa_plugin(rtobject_t* rtobj, rtobject_instance_t* rtins){
  ladspa_plugin_t *plugin_struct;
  const LADSPA_Descriptor* plugin_descriptor = \
    ((ladspa_plugin_t*)rtobj->imp_struct)->LADSPA_Descriptor;
 
  plugin_struct = (ladspa_plugin_t*)rtobj->imp_struct;

  /*the instance may need access to the LADSPA descriptor in order to
    change port connections later*/
  rtins->imp_data.ladspa_plugin_element.descriptor = plugin_descriptor;

  /*note whether plugin is running in add or overwrite mode*/
  rtins->imp_data.ladspa_plugin_element.overwrite = (!plugin_struct->use_run_adding_emulation);

  /*LADSPA API CALL*/
  /*instantiate a plugin instance through the LADSPA API*/
  if (!(rtins->imp_data.ladspa_plugin_element.handle = \
	plugin_descriptor->instantiate(plugin_descriptor, soundtank_engine->sample_rate))){

    printf("ladspa instance error: failed to instantiate new plugin instance\n");
    rtins->imp_data.ladspa_plugin_element.descriptor = 0;
    return -1;
  }

  /*attach LADSPA plugin ports to Soundtank buffers and control values*/
  if ((attach_instance_ladspa_plugin(rtobj,rtins)) < 0){
    printf("ladspa instance error: failed attempt to attach new plugin instance\n");
    rtins->imp_data.ladspa_plugin_element.descriptor = 0;
    return -1;
  }

  return 0;
}


int deinit_instance_ladspa_plugin(rtobject_t* rtobj, rtobject_instance_t* rtins){
  ladspa_plugin_t* plugin_struct;

  plugin_struct = (ladspa_plugin_t*)rtobj->imp_struct;

  plugin_struct->LADSPA_Descriptor->cleanup(rtins->imp_data.ladspa_plugin_element.handle);

  return 0;
}



int attach_instance_ladspa_plugin(rtobject_t* rtobj, rtobject_instance_t* rtins){
  int i;
  data_port_t* curr_port; 
  const LADSPA_Descriptor* plugin_descriptor = \
    ((ladspa_plugin_t*)rtobj->imp_struct)->LADSPA_Descriptor;

  if (debug_readout) printf("attaching ladspa plugin\n");

  /*LADSPA API CALL*/
  /*attach LADSPA plugin data ports to channel buffers or working buffers*/
  for (i=0;i<rtobject_get_data_port_list_size(rtobj);++i){

    /*get data port from rtobject*/
    if (!(curr_port = rtobject_get_data_port(rtobj,i))){
      printf("attach instance ladspa plugin error: couldn't get data port %d\n",i);
      return -1;
    }

    /*make sure instance has been initialized properly*/
    if (!rtins->data_port_list[i]){
      printf("error: LADSPA plugins can't deal with ports that aren't attached to anything\n");
      return -1;
    }

    /*if available, attach to working buffer for non-destructive mixing into a channel*/
      if (curr_port->working_buffer){

	plugin_descriptor->connect_port(rtins->imp_data.ladspa_plugin_element.handle,
					curr_port->ladspa_port,
					curr_port->working_buffer->data
					);
				      
      }else{

	plugin_descriptor->connect_port(rtins->imp_data.ladspa_plugin_element.handle,
					curr_port->ladspa_port,		       
					rtins->data_port_list[i]->data
					);
      }

  }

  /*LADSPA API CALL*/
  /*attach LADSPA plugin control ports to control values*/
  for (i=0;i<rtobject_get_control_list_size(rtobj);++i){
    const control_t* curr_control = rtobject_get_control(rtobj, i);

    if (!curr_control){
      printf("error: LADSPA plugin can't find control %d\n", i);
      return -1;
    }
    
    if (control_belongs_to_ladspa(curr_control))
      plugin_descriptor->connect_port(rtins->imp_data.ladspa_plugin_element.handle,
				      control_get_ladspa_port(curr_control),
				      (sample_t*)&rtins->control_list[i]
				      );
    
  }
  
  /*remake instance's output data port list: used for lazy-zeroing & LADSPA run-adding emulation*/
  /*clear out any old data*/
  while (rtins->imp_data.ladspa_plugin_element.output_data_port_list){
    ll_remove(rtins->imp_data.ladspa_plugin_element.output_data_port_list,\
	      &rtins->imp_data.ladspa_plugin_element.output_data_port_list);
  }
  

  for (i=0;i<rtobject_get_data_port_list_size(rtobj);++i){
    curr_port = rtobject_get_data_port(rtobj,i);
    if (!(data_port_get_input(curr_port))){
      if (!(ll_append(&rtins->imp_data.ladspa_plugin_element.output_data_port_list,\
		      (void*)curr_port))){
	printf("memory error\n");
	return -1;
      }
    }
  }
 

  return 0;
}

int detach_instance_ladspa_plugin(rtobject_t* rtobj, rtobject_instance_t* rtins){
  int i;
  ladspa_plugin_t* plugin_struct;
  data_port_t* curr_port;

  plugin_struct = (ladspa_plugin_t*)rtobj->imp_struct;

  for (i=0;i<rtobject_get_data_port_list_size(rtobj);++i){
    curr_port = rtobject_get_data_port(rtobj,i);

    /*TODO: I don't think calling LADSPA connect with a null buffer is legal*/
    plugin_struct->LADSPA_Descriptor->connect_port(rtins->imp_data.ladspa_plugin_element.handle,
						       curr_port->ladspa_port,
						       null_write->buffer_struct->data
						       );

  }

  return 0;
}






/*TODO: move this to ladspa_search.c*/
void ladspa_plugin_loading_search(const char * pcFullFilename, 
		      void * pvPluginHandle,
		      LADSPA_Descriptor_Function fDescriptorFunction) {

  const LADSPA_Descriptor * psDescriptor;
  long lIndex;
  
  
  for (lIndex = 0;
       (psDescriptor = fDescriptorFunction(lIndex)) != NULL;
       lIndex++){

    if (((load_id)&&(psDescriptor->UniqueID == load_id))||\
	((!load_id)&&(load_label)&&(!strcmp(psDescriptor->Label,load_label)))){


      if (load_imp_object->LADSPA_Descriptor){

	printf("load ladspa error: you did not uniquely identify a plugin, first matching plugin was opened\n");

      }else{

	if (debug_readout){
	  printf("loading LADSPA plugin:\n");
	  printf("%s:\n", pcFullFilename);
	  printf("\t%s (%lu/%s)\n", 
		 psDescriptor->Name,
		 psDescriptor->UniqueID,
		 psDescriptor->Label);
	}

	load_imp_object->LADSPA_Plugin_Library = pvPluginHandle;
	load_imp_object->LADSPA_Descriptor = psDescriptor;
	
	/*returning now leaves the library open which is what we want*/
	return;
      }

    }

  }

  dlclose(pvPluginHandle);

}


void print_imp_object_ladspa_plugin(rtobject_t* rtobj){
  ladspa_plugin_t *plugin_struct;

  /*sanity check, make sure imp type is ladspa plugin*/
  if (rtobject_get_implementation_type(rtobj) != RTOBJECT_IMP_TYPE_LADSPA_PLUGIN){
    printf("print ladspa plugin error: rtobject not a LADSPA plugin\n");
    return;
  }

  plugin_struct = (ladspa_plugin_t*)rtobj->imp_struct;

  /*print plugin label*/
  printf("[ label:%s  ", plugin_struct->LADSPA_Descriptor->Label);

  /*print plugin id*/
  printf("id:%ld  ", plugin_struct->LADSPA_Descriptor->UniqueID);


  /*display overwrite mode*/
  if (plugin_struct->use_run_adding_emulation){

    printf("additive ");

  }else{

    printf("overwrite ");

  }

  printf("] ");

}
