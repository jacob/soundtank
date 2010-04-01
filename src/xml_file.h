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

#ifndef SOUNDTANK_XML_FILE_INCLUDE
#define SOUNDTANK_XML_FILE_INCLUDE

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <iconv.h>

int load_soundtank_xml_file(const char* pathname);

xmlNodePtr xml_rtobject_node_get_loaded(xmlNodePtr rtobj_node);
rtobject_t* rtobject_load_from_xml(xmlNodePtr rtobj_node, ll_head file_node_list);

int rtobject_save_to_file(rtobject_t* rtobj, const char *file_name);
int rtobject_save_to_xml(rtobject_t* rtobj, xmlNodePtr rtobject_node);


#endif
