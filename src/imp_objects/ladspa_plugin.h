/*
 *  ladspa plugin wrapper rtobject code
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



typedef struct ladspa_plugin_struct ladspa_plugin_t;
struct ladspa_plugin_struct{

  void* LADSPA_Plugin_Library;
  const LADSPA_Descriptor* LADSPA_Descriptor;
  int use_run_adding_emulation;

};




ladspa_plugin_t*  imp_object_ladspa_plugin_alloca();
void imp_object_ladspa_plugin_dealloca(ladspa_plugin_t* oldobj);

int create_imp_object_ladspa_plugin(rtobject_t* rtobj);
int destroy_imp_object_ladspa_plugin(rtobject_t* rtobj);


int init_instance_ladspa_plugin(rtobject_t* rtobj, rtobject_instance_t* rtins);
int deinit_instance_ladspa_plugin(rtobject_t* rtobj, rtobject_instance_t* rtins);

int attach_instance_ladspa_plugin(rtobject_t* rtobj, rtobject_instance_t* rtins);
int detach_instance_ladspa_plugin(rtobject_t* rtobj, rtobject_instance_t* rtins);

void ladspa_plugin_loading_search(const char * pcFullFilename, 
				  void * pvPluginHandle,
				  LADSPA_Descriptor_Function fDescriptorFunction);

void print_imp_object_ladspa_plugin(rtobject_t* rtobj);
