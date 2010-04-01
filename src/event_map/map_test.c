/*
 * event map test element code
 *
 * Copyright 2004 Jacob Robbins
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


#include <stdio.h>

#include "../include.h"


void map_test_clear(map_test_t* test){
  
  test->arg1 = test->arg2 = test->arg3 = 0;

  test->fxn = 0;

  map_test_clear_action_list(test);

}

void map_test_print(const map_test_t* test, rtobject_t *rtobj){
  int i;
  char* ret;
  
  ret = map_test_get_func_name(test);
  printf("   %s (", ret);
  free(ret);

  i = 0;
  while ((ret = map_test_get_argv(test, i++, rtobj))){

    if (i>1) printf(",");
    printf(" %s", ret);
    free(ret);

  }
  
  printf(" )  %d actions: ", map_test_get_action_list_size(test));
  
  for (i=0;i<map_test_get_action_list_size(test);++i){
    if (i>0) printf("; ");
    map_action_print(map_test_get_action(test,i), rtobj);
  }

  printf("\n");

}

void map_test_copy(map_test_t* dest, const map_test_t* src){
  /*does not allocate dest, but does allocate copied actions*/
  node_t* temp_node;
  map_action_t* curr_action;

  dest->arg1 = src->arg1;
  dest->arg2 = src->arg2;
  dest->arg3 = src->arg3;

  dest->fxn = src->fxn;

  /*make sure to zero out new action list before appending to it*/
  dest->action_list = 0;

  /*copy action list, making copies of all actions*/
  for (temp_node=src->action_list;temp_node;temp_node=temp_node->next){
    
    curr_action = map_action_copy((map_action_t*)temp_node->data);

    if (curr_action)
      map_test_insert_action(dest, curr_action, -1);

  }

}

int map_test_get_action_list_size(const map_test_t* test){

  return ll_get_size(&test->action_list);
}

map_action_t* map_test_get_action(const map_test_t* test, int pos){
  node_t* temp_node;
  int i;

  if (!(temp_node = test->action_list)) return 0;

  for (i=0;i<pos;++i){
    
    if (!(temp_node=temp_node->next)) return 0;

  }
  
  return (map_action_t*)temp_node->data;
}

int map_test_insert_action(map_test_t* test, map_action_t* action, int pos){

  if (pos < 0){
    /*append*/

    if (!(ll_append(&test->action_list, (void*)action)))
      return -1;

  }else if (pos == 0){
    /*prepend*/

    if (!(ll_prepend(&test->action_list, (void*)action)))
      return -1;

  }else{
    /*insert after action at position pos*/
    int i;
    node_t* temp_node;

    /*start at head of action list*/
    if (!(temp_node = test->action_list)){

      /*list is empty so just append new action*/
      if (!(ll_append(&test->action_list, (void*)action)))
	return -1;
      return 0;

    }
    
    /*go through list but don't fall off end*/
    /*note that pos is not zero based in this situation*/
    for (i=1;i<pos;++i){
      
      if (temp_node->next) temp_node = temp_node->next;

    }

    if (!(ll_insert_after(temp_node,(void*)action)))
      return -1;

  }

  return 0;
}


int map_test_delete_action(map_test_t* test, int pos){
  node_t* temp_node;
  int i;

  if (!(temp_node = test->action_list)) return -1;

  for (i=0;i<pos;++i){
    
    if (!(temp_node=temp_node->next)) return -1;

  }
  
  map_action_dealloca((map_action_t*)temp_node->data);

  ll_remove(temp_node, &test->action_list);
  
  return 0;
}

void map_test_clear_action_list(map_test_t* test){

  while (test->action_list){

    map_action_dealloca((map_action_t*)test->action_list->data);
    ll_remove(test->action_list, &test->action_list);

  }

}

int map_test_save_to_xml(const map_test_t* test, xmlNodePtr* xml_node, rtobject_t *rtobj){
  int i;
  char *ret;
  xmlAttrPtr attp; 

  /*add indentation for human readability*/
  xmlAddChild((*xml_node),xmlNewText("\n    "));

  /*add test node*/
  if (!((*xml_node) = xmlNewChild((*xml_node),NULL,"MapTest",0))){
    printf("map test xml error: unable to create node MapTest\n");  
    return -1;
  }

  /*save name of callback fxn*/
  if (!(ret = map_test_get_func_name(test))){
    printf("map test xml error: couldn't find callback function name\n");
    return -1;
  }
  if (!(attp = xmlSetProp((*xml_node),"Name",ret))){
    printf("map test xml error: unable to set attribute: Name\n");
    return -1;
  }
  free(ret);

  /*add initiation args as child nodes*/
  i = 0;
  while ((ret = map_test_get_argv(test, i++, rtobj))){

    if (!((*xml_node) = xmlNewChild((*xml_node),NULL,"ImpArg",0))){
      printf("map test xml error: unable to create node ImpArg\n");
      return -1;
    }

    if (!(attp = xmlSetProp((*xml_node),"Value", ret))){
      printf("map test xml error: unable to set attribute: Arg Value\n");        
      return -1;
    }

    free(ret);

    (*xml_node) = (*xml_node)->parent;

  }

  /*add actions*/
  for (i=0;i<map_test_get_action_list_size(test);++i){

    if (map_action_save_to_xml(map_test_get_action(test, i), xml_node, rtobj) < 0){
      printf("map test xml error: unable to save action %d\n", i);
      return -1;
    }

  }

  /*put node pointer back where it started*/
  (*xml_node) = (*xml_node)->parent;

  /*add indentation for human readability*/
  xmlAddChild((*xml_node),xmlNewText("\n    "));

  return 0;
}

map_test_t* map_test_load_from_xml(xmlNodePtr* xml_node, rtobject_t* rtobj){
  xmlNodePtr temp_ptr;
  map_test_t* new_test;
  map_action_t* new_action;
  int argc, i;
  char **argv;

  /*make sure we have an MapTest node*/
  if (strcmp((char*)(*xml_node)->name,"MapTest")){
    printf("map test xml error: not an MapTest node\n");
    return 0;
  }

  /*create new test*/
  if (!(new_test = (map_test_t*)malloc(sizeof(map_test_t)))){
    printf("map test xml error: couldn't create new test\n");
    return 0;
  }
  /*this is a pain in the ass*/
  memset((void*)new_test, 0, sizeof(map_test_t));

  /*count init args*/
  argc = 1; /*first arg is callback fxn name*/
  for (temp_ptr=(*xml_node)->children;temp_ptr;temp_ptr=temp_ptr->next)
    if (!(strcmp((char*)temp_ptr->name,"ImpArg")))
      ++argc;

  /*make and fill argv*/
  if (!(argv = (char**)malloc((argc)*sizeof(char*)))){
    printf("map test xml error: memory error\n");
    return 0;
  }

  if (!(argv[0] = strdup((char*)xmlGetProp((*xml_node), "Name")))){
    printf("map test xml error: memory error\n");
    return 0;
  }
  
  i = 1;
  for (temp_ptr=(*xml_node)->children;temp_ptr;temp_ptr=temp_ptr->next){

    if (!(strcmp((char*)temp_ptr->name,"ImpArg"))){
      
      if (!(argv[i++] =  strdup((char*)xmlGetProp(temp_ptr, "Value")))){
	printf("map test xml error: memory error\n");
	return 0;
      }

    }

  }

  /*init test*/
  {
    ev_route_frame_t frame;
    frame.rtobj = rtobj;
    frame.test = new_test;

    if (map_test_init(&frame, argc, argv) < 0){
      printf("map test xml error: couldn't initialize new test\n");
      return 0;
    }

  }

  /*load actions*/
  for (temp_ptr=(*xml_node)->children;temp_ptr;temp_ptr=temp_ptr->next){

    if (!(strcmp((char*)temp_ptr->name,"MapAction"))){

      /*create action*/
      if (!(new_action = map_action_load_from_xml(&temp_ptr, rtobj))){
	printf("map test xml error: couldn't load an action, continuing\n");
      }else{

	/*add new action to test's action list*/
	if (map_test_insert_action(new_test, new_action, -1) < 0){
	  printf("map test xml error: couldn't add new action to test\n");
	  return 0;
	}

      }

    }

  }

  return new_test;
}

