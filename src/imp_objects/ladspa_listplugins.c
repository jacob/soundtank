/* listplugins.c

   Free software by Richard W.E. Furse. Do with as you will. No
   warranty. */

/* This file is part of the LADSPA SDK, with some additions by Jacob Robbins. */
/* Additions to this particular file by Jacob Robbins are released under the same license as the LADSPA SDK.*/

/*****************************************************************************/

#include <dlfcn.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/*****************************************************************************/

#include <ladspa.h>
#include "../include.h"


unsigned long search_id;
const char* search_label;

/*****************************************************************************/

void set_ladspa_search_id(unsigned long new_search_id){
  search_id = new_search_id;
}
void set_ladspa_search_label(const char* new_search_label){
  search_label = new_search_label;
}

void describePluginLibrary(const char * pcFullFilename, 
		      void * pvPluginHandle,
		      LADSPA_Descriptor_Function fDescriptorFunction) {

  const LADSPA_Descriptor * psDescriptor;
  long lIndex;

  printf("%s:\n", pcFullFilename);
  for (lIndex = 0;
       (psDescriptor = fDescriptorFunction(lIndex)) != NULL;
       lIndex++) 
    printf("\t%s (%lu/%s)\n", 
	   psDescriptor->Name,
	   psDescriptor->UniqueID,
	   psDescriptor->Label);

  dlclose(pvPluginHandle);
}




/***The following are additions by Jacob Robbins***/


/*****************************************************************************/
void idsearchPluginLibrary(const char * pcFullFilename, 
		      void * pvPluginHandle,
		      LADSPA_Descriptor_Function fDescriptorFunction) {

  const LADSPA_Descriptor * psDescriptor;
  long lIndex;

  for (lIndex = 0;
       (psDescriptor = fDescriptorFunction(lIndex)) != NULL;
       lIndex++) 
    
    if (psDescriptor->UniqueID == search_id){
      printf("%s:\n", pcFullFilename);
      printf("\t%s (%lu/%s)\n", 
	     psDescriptor->Name,
	     psDescriptor->UniqueID,
	     psDescriptor->Label);
      
    }

  dlclose(pvPluginHandle);

}
/*****************************************************************************/
void labelsearchPluginLibrary(const char * pcFullFilename, 
		      void * pvPluginHandle,
		      LADSPA_Descriptor_Function fDescriptorFunction) {

  const LADSPA_Descriptor * psDescriptor;
  long lIndex;

  if (!search_label){
    dlclose(pvPluginHandle);
    return;
  }

  for (lIndex = 0;
       (psDescriptor = fDescriptorFunction(lIndex)) != NULL;
       lIndex++){

    if (!strcmp(psDescriptor->Label,search_label)){
      printf("%s:\n", pcFullFilename);
      printf("\t%s (%lu/%s)\n", 
	     psDescriptor->Name,
	     psDescriptor->UniqueID,
	     psDescriptor->Label);
    }
  }

  dlclose(pvPluginHandle);

}
/*****************************************************************************/
void approposearchPluginLibrary(const char * pcFullFilename, 
		      void * pvPluginHandle,
		      LADSPA_Descriptor_Function fDescriptorFunction) {

  /*search to see if search string is in name or label of LADSPA plugin*/
  /*case-blind search, compares lower-case copies of all strings*/
  const LADSPA_Descriptor * psDescriptor;
  long lIndex;
  char * lower_search_label;
  char* lower_name;
  char* lower_label;
  int i;
  

  if (!search_label){
    dlclose(pvPluginHandle);
    return;
  }

  if (!(lower_search_label = string_to_lower(search_label))){
    printf("memory error: search incomplete\n");
    dlclose(pvPluginHandle);
    return;
  }

  for (lIndex = 0;
       (psDescriptor = fDescriptorFunction(lIndex)) != NULL;
       lIndex++){
    
    /*make lower case copy of name*/
    if (!(lower_name = (char*)malloc((1+strlen(psDescriptor->Name))*sizeof(char)))){
      printf("memory error: search incomplete\n");
      dlclose(pvPluginHandle);
      return;
    }
    for (i=0;i<strlen(psDescriptor->Name);++i)
      lower_name[i] = (char)tolower(psDescriptor->Name[i]);
    lower_name[strlen(psDescriptor->Name)] = '\0';

    /*make lower case copy of label*/
    if (!(lower_label = (char*)malloc((1+strlen(psDescriptor->Label))*sizeof(char)))){
      printf("memory error: search incomplete\n");
      dlclose(pvPluginHandle);
      return;
    }
    for (i=0;i<strlen(psDescriptor->Label);++i)
      lower_label[i] = (char)tolower(psDescriptor->Label[i]);
    lower_label[strlen(psDescriptor->Label)] = '\0';

    if ((strstr(lower_name,lower_search_label))||(strstr(lower_label,lower_search_label))){

      printf("\na match was found between %s, %s, and %s\n",lower_name,lower_label,lower_search_label);

      printf("%s:\n", pcFullFilename);
      printf("\t%s (%lu/%s)\n", 
	     psDescriptor->Name,
	     psDescriptor->UniqueID,
	     psDescriptor->Label);
    }

    free(lower_name);
    free(lower_label);

  }

  free(lower_search_label);

  dlclose(pvPluginHandle);

}

/*****************************************************************************/


/* EOF */
