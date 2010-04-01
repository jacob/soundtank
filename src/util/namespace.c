/*
 * simple namespace code
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
#include <string.h>

#include "../include.h"

extern int debug_readout; 


/*create a namespace*/
namespace_t* create_namespace(ll_head *members, ns_node_name_callback cb){
  namespace_t *ns;

  if (!(ns = (namespace_t*)malloc(sizeof(namespace_t)))){
    return 0;
  }

  ns->members = members;
  ns->cb = cb;

  return ns;
}

/*destroy a namespace*/
void destroy_namespace(namespace_t* ns){
  free(ns);
}

/*get the linked list of members*/
ll_head ns_get_member_list(namespace_t* ns){
  return *ns->members;
}

/*find the name of the node at a certain position*/
const char* ns_get_name(namespace_t *ns, int pos){
  node_t* temp_node;

  if (!(temp_node = ll_get_node(ns->members, pos))){
    return 0;
  }

  return ns->cb(temp_node);
}

/*make a new unique name from a seed name, allocating memory*/
char* ns_make_unique_name(namespace_t *ns, const char* name){
  node_t* temp_node;
  char *curr_name, *curr_postfix_string;
  int curr_postfix;

  /*first see if we can get desired name*/
  if (!(temp_node = ns_search(ns, name, 0, SEARCH_EXACT))){

    if (!(curr_name = strdup(name))){
      printf("memory error\n");
      return 0;
    }

    return curr_name;
  }

  /*keep adding numbers onto the end until we find an unused name*/
  /*gentle reader, this sort of operation is not my forte therefore
    I ask that you remain strong while reading the following*/
  
  curr_postfix = 1; /*start with "name2"*/

  if (!(curr_postfix_string = (char*)malloc(12*sizeof(char)))){
    printf("memory error\n");
    return 0;
  }

  if (!(curr_name = (char*)malloc((12+strlen(name))*sizeof(char)))){
    printf("memory error\n");
    return 0;
  }

  while (1){

    /*increment postfix*/
    snprintf(curr_postfix_string,12,"%d",++curr_postfix);

    /*cat name & postfix > curr_name*/
    strcpy(curr_name,name);
    strcat(curr_name,curr_postfix_string);
    
    /*check curr_name*/
    if (!(temp_node = ns_search(ns, curr_name, 0, SEARCH_EXACT))){

      if (debug_readout)  printf("using unique name: %s\n",curr_name);

      free(curr_postfix_string);

      return curr_name;
    }
   
  }

  return 0;
}

/*NOTES ABOUT SEARCHES: searches are not case-sensitive. searches can
  return more than 1 node. when begining a search, pass state=0, pass
  incremented state to continue search to additional matches. pass a
  search option giving the type of search*/

/*
search for a name: SEARCH_EXACT 
search for a substring: SEARCH_SUBSTRING 
search for a substring that starts at the beginning
of the name: SEARCH_INITIAL_SUBSTRING
*/

node_t* ns_search(namespace_t* ns, const char* name, int state, int type){
  node_t* temp_node;
  int i, match;
  char *name_lower;

  if (!(name_lower = string_to_lower(name))){
    printf("memory error\n");
    return 0;
  }

  i = 0;

  for (temp_node=*(ns->members); temp_node; temp_node=temp_node->next){
    char *node_name_lower;

    if (!(node_name_lower = string_to_lower(ns->cb(temp_node)))){
      printf("memory error\n");
      free(name_lower);
      return 0;
    }

    match = 0;

    switch (type){

    case SEARCH_EXACT:

      if (!(strcmp(name_lower, node_name_lower))) 
	match = 1;
      break;

    case SEARCH_SUBSTRING:

      if (!(strstr(node_name_lower, name_lower))) 
	match = 1;
      break;

    case SEARCH_INITIAL_SUBSTRING:
      if ((node_name_lower) == (strstr(node_name_lower, name_lower)))
	match = 1;
      break;

    default:
      break;
    }

    free(node_name_lower);

    if ((match)&&(i++ >= state)){
      free(name_lower);
      return temp_node;
    }


  }

  free(name_lower);
  
  return 0;
}


int ns_search_pos(namespace_t* ns, const char* name, int state, int type){
  node_t* temp_node;
  int i, match, pos;
  char *name_lower;

  if (!(name_lower = string_to_lower(name))){
    printf("memory error\n");
    return -1;
  }

  pos = i = 0;

  for (temp_node=*(ns->members); temp_node; temp_node=temp_node->next){
    char *node_name_lower;

    if (!(node_name_lower = string_to_lower(ns->cb(temp_node)))){
      printf("memory error\n");
      free(name_lower);
      return -1;
    }

    match = 0;

    switch (type){

    case SEARCH_EXACT:

      if (!(strcmp(name_lower, node_name_lower))) 
	match = 1;
      break;

    case SEARCH_SUBSTRING:

      if (!(strstr(node_name_lower, name_lower))) 
	match = 1;
      break;

    case SEARCH_INITIAL_SUBSTRING:
      if ((node_name_lower) == (strstr(node_name_lower, name_lower)))
	match = 1;
      break;

    default:
      break;
    }

    free(node_name_lower);

    if ((match)&&(i++ >= state)){
      free(name_lower);
      return pos;
    }

    ++pos;

  }

  free(name_lower);
  
  return -1;
}
