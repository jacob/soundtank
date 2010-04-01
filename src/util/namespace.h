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

#ifndef SOUNDTANK_NAMESPACE_INTERFACE_INCLUDE
#define SOUNDTANK_NAMESPACE_INTERFACE_INCLUDE


/*
Namespace is a high level interface to linked list. As such, it takes
a linked-list and a callback fxn that gets the name of a node. This
replaces the unique name c module here in util.
*/

typedef const char* (*ns_node_name_callback)(node_t* node);

typedef struct namespace namespace_t;
struct namespace{

  ll_head *members;
  ns_node_name_callback cb;

};

/*create a namespace*/
namespace_t* create_namespace(ll_head *members, ns_node_name_callback cb);

/*destroy a namespace*/
void destroy_namespace(namespace_t* ns);

/*get the linked list of members*/
ll_head ns_get_member_list(namespace_t* ns);

/*find the name of the node at a certain position*/
const char* ns_get_name(namespace_t *ns, int pos);

/*make a new unique name from a seed name, allocating memory*/
char* ns_make_unique_name(namespace_t *ns, const char* name);


/*NOTES ABOUT SEARCHES: searches are not case-sensitive. searches can
  return more than 1 node. when begining a search, pass state=0, pass
  incremented state to continue search to additional matches. node
  search exhausted when fxn returns NULL. position search exhausted
  when fxn returns negative value. errors also return NULL or negative
  so you can't tell if a problem has occured. pass a search type from
  the defines*/

#define SEARCH_EXACT 1
#define SEARCH_SUBSTRING 2
#define SEARCH_INITIAL_SUBSTRING 3

node_t* ns_search(namespace_t* ns, const char* name, int state, int type);

int ns_search_pos(namespace_t* ns, const char* name, int state, int type);

#endif
