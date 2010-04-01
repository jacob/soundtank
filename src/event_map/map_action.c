
#include <string.h>

#include "../include.h"


void map_action_print(map_action_t* action, rtobject_t *rtobj){
  int i;
  char* ret;
  
  ret = map_action_get_func_name(action);
  printf(" %s (", ret);
  free(ret);

  i = 0;
  while ((ret = map_action_get_argv(action, i++, rtobj))){

    if (i>1) printf(",");
    printf(" %s", ret);
    free(ret);

  }
  
  printf(" )");
}

map_action_t* map_action_alloca(){
  map_action_t* new_action;

  if (!(new_action = (map_action_t*)malloc(sizeof(map_action_t)))){
    return 0;
  }
  
  /*must initialize this way to zero out the args union*/
  memset((void*)new_action, 0, sizeof(map_action_t));

  return new_action;
}


void map_action_dealloca(map_action_t* action){
  free(action);
}


map_action_t* map_action_copy(map_action_t* src){
  /*allocates memory*/
  map_action_t* new_action; 
  
  if (!(new_action = map_action_alloca())){
    return 0;
  }
  
  /*must do it this way to copy the args union*/
  memcpy((void*)new_action, (void*)src, sizeof(map_action_t));
 
  return new_action;
}


int map_action_save_to_xml(map_action_t* action, xmlNodePtr* xml_node, rtobject_t *rtobj){
  int i;
  char *ret;
  xmlAttrPtr attp; 

  /*add indentation for human readability*/
  xmlAddChild((*xml_node),xmlNewText("\n       "));

  /*add action node*/
  if (!((*xml_node) = xmlNewChild((*xml_node),NULL,"MapAction",0))){
    printf("map action xml error: unable to create node MapAction\n");  
    return -1;
  }

  /*save name of callback fxn*/
  if (!(ret = map_action_get_func_name(action))){
    printf("map action xml error: couldn't find callback function name\n");
    return -1;
  }
  if (!(attp = xmlSetProp((*xml_node),"Name",ret))){
    printf("map action xml error: unable to set attribute: Name\n");
    return -1;
  }
  free(ret);

  /*add initiation args as child nodes*/
  i = 0;
  while ((ret = map_action_get_argv(action, i++, rtobj))){

    if (!((*xml_node) = xmlNewChild((*xml_node),NULL,"ImpArg",0))){
      printf("map action xml error: unable to create node ImpArg\n");
      return -1;
    }

    if (!(attp = xmlSetProp((*xml_node),"Value", ret))){
      printf("map action xml error: unable to set attribute: Arg Value\n");    
      return -1;
    }

    free(ret);

    (*xml_node) = (*xml_node)->parent;

  }

  /*put node pointer back where it started*/
  (*xml_node) = (*xml_node)->parent;

  return 0;
}

map_action_t* map_action_load_from_xml(xmlNodePtr* xml_node, rtobject_t* rtobj){
  xmlNodePtr temp_ptr;
  map_action_t* new_action;
  int argc, i;
  char **argv;

  /*make sure we have a MapAction node*/
  if (strcmp((char*)(*xml_node)->name,"MapAction")){
    printf("map action xml error: not an MapAction node\n");
    return 0;
  }

  /*count init args*/
  argc = 1; /*first arg is callback fxn name*/
  for (temp_ptr=(*xml_node)->children;temp_ptr;temp_ptr=temp_ptr->next)
    if (!(strcmp((char*)temp_ptr->name,"ImpArg")))
      ++argc;

  /*make and fill argv*/
  if (!(argv = (char**)malloc((argc)*sizeof(char*)))){
    printf("map action xml error: memory error\n");
    return 0;
  }

  if (!(argv[0] = strdup((char*)xmlGetProp((*xml_node), "Name")))){
    printf("map action xml error: memory error\n");
    return 0;
  }
  
  i = 1;
  for (temp_ptr=(*xml_node)->children;temp_ptr;temp_ptr=temp_ptr->next){

    if (!(strcmp((char*)temp_ptr->name,"ImpArg"))){
      
      if (!(argv[i++] =  strdup((char*)xmlGetProp(temp_ptr, "Value")))){
	printf("map action xml error: memory error\n");
	return 0;
      }

    }

  }

  /*create & initialize action*/
  {
    ev_route_frame_t frame;
    frame.rtobj = rtobj;

    if (map_action_init(&frame, argc, argv) < 0){
      printf("map action xml error: couldn't initialize new action\n");
      return 0;
    }

    new_action = frame.action;

  }

  return new_action;
}
