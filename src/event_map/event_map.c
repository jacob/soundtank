

#include "../include.h"




event_map_t* event_map_alloca(){
  event_map_t* new_map;

  if (!(new_map = (event_map_t*)malloc(sizeof(event_map_t)))) return 0;

  if (!(new_map->test_array = generic_array_create(sizeof(map_test_t)))){
    free(new_map);
    return 0;
  }

  /*test array must be terminated by an empty test or things go to pot*/
  if ((generic_array_append_element(new_map->test_array, 0)) < 0){
    free(new_map);
    return 0;
  }

  return new_map;
}


void event_map_dealloca(event_map_t* map){

  event_map_clear_test_list(map);

  generic_array_destroy(map->test_array);

  free(map);
}


event_map_t* event_map_copy(event_map_t* src){
  /*allocates memory*/
  int i;
  event_map_t* new_map;

  if (!(new_map = event_map_alloca())){
    return 0;
  }

  /*because map_test_copy doesn't allocate a new test, we must first
    set the size of the array and then copy the tests*/
  if ((generic_array_set_size(new_map->test_array,\
			      generic_array_get_size(src->test_array))) < 0){
    event_map_dealloca(new_map);
    return 0;
  }

  for (i=0;i<generic_array_get_size(new_map->test_array);++i){

    map_test_copy((map_test_t*)generic_array_get_element_pointer(new_map->test_array,i),\
		  (map_test_t*)generic_array_get_element_pointer(src->test_array,i));
    
  }

  return new_map;
}

int event_map_get_test_list_size(const event_map_t* map){
  /*we don't count the terminating empty test*/
  return (generic_array_get_size(map->test_array) - 1);
}

map_test_t* event_map_get_test(const event_map_t* map, int pos){

  /*you can't touch the terminating empty test*/
  if (pos == event_map_get_test_list_size(map)) return 0;

  return (map_test_t*)generic_array_get_element_pointer(map->test_array, pos);  
}

int event_map_insert_test(event_map_t* map, map_test_t* test, int pos){

  if (pos < 0){
    /*append (but actually insert before terminating empty test)*/
    if ((generic_array_insert_element(map->test_array, \
				      event_map_get_test_list_size(map),\
				      (void*)test)) < 0)
      return -1;
    
  }else{
    /*insert*/
    if ((generic_array_insert_element(map->test_array, pos, (void*)test)) < 0)
      return -1;

  }

  return 0;
}

int event_map_delete_test(event_map_t* map, int pos){

  /*can't delete terminating empty test*/
  if (pos >= event_map_get_test_list_size(map)) return -1;

  return generic_array_remove_element(map->test_array, pos, 0);
}


void event_map_clear_test_list(event_map_t* map){

  while ((event_map_get_test_list_size(map)) > 0)
    event_map_delete_test(map, 0);
}

void event_map_print(event_map_t* map, rtobject_t *rtobj){
  int i;

  for (i=0;i<event_map_get_test_list_size(map);++i)
    map_test_print(event_map_get_test(map, i), rtobj);

}

int event_map_save_to_xml(const event_map_t* map, xmlNodePtr* xml_node,\
			  rtobject_t *rtobj){
  int i;

  /*add indentation for human readability*/
  xmlAddChild((*xml_node),xmlNewText("\n  "));

  /*add map node*/
  if (!((*xml_node) = xmlNewChild((*xml_node),NULL,"EventMap",0))){
    printf("event map xml error: unable to create node EventMap\n");  
    return -1;
  }

  /*add tests*/
  for (i=0;i<event_map_get_test_list_size(map);++i){

    if (map_test_save_to_xml(event_map_get_test(map, i), xml_node, rtobj) < 0){
      printf("event map xml error: unable to save test %d\n", i);
      return -1;
    }

  }

  /*put node pointer back where it started*/
  (*xml_node) = (*xml_node)->parent;

  return 0;
}

event_map_t* event_map_load_from_xml(xmlNodePtr* xml_node, rtobject_t* rtobj){
  event_map_t* new_map;
  xmlNodePtr temp_ptr;
  map_test_t* new_test;

  /*make sure we have an EventMap node*/
  if (strcmp((char*)(*xml_node)->name,"EventMap")){
    printf("event map xml error: not an EventMap node\n");
    return 0;
  }

  /*create new map*/
  if (!(new_map = event_map_alloca())){
    printf("event map xml error: couldn't create a new event map\n");
    return 0;
  }

  /*load tests*/
  for (temp_ptr=(*xml_node)->children;temp_ptr;temp_ptr=temp_ptr->next){

    if (!(strcmp((char*)temp_ptr->name,"MapTest"))){

      /*create test from xml*/
      if (!(new_test = map_test_load_from_xml(&temp_ptr, rtobj))){
	printf("event map xml error: couldn't load one of the tests, ");
	printf("continuing anyway\n");
      }else{

	/*have test copied into the map*/
	if (event_map_insert_test(new_map, new_test, -1) < 0){
	  printf("event map xml error: couldn't insert new test\n");
	  free(new_map);
	  event_map_dealloca(new_map);
	  return 0;
	}

	/*free newly created test, it's been copied into the map*/
	free(new_test);

      }

    }

  }

  return new_map;
}


/*rtobject functions: (TODO: put these elsewhere maybe?)*/

int rtobject_get_map_list_size(rtobject_t* rtobj){
  return generic_array_get_size(rtobj->event_map_list);
}


int rtobject_get_map(rtobject_t* rtobj, event_map_t** ptr, int pos){
  void* ptr_addr;
  if (!(ptr_addr = generic_array_get_element_pointer(rtobj->event_map_list, pos)))
    return -1;

  *ptr = *((event_map_t**)ptr_addr);

  return 0;
}

/*generic procedure for changing rtobject's map list:
  -1- make copy of map list (or test list)
  -2- modify copy
  -3- do pointer_hot_swap(original, copy)
  -4- deallocate old struct (now referenced by copy)
*/


int rtobject_insert_map(rtobject_t* rtobj, event_map_t* map, int pos){
  generic_array_t* dup_map_list;
  
  if (!(dup_map_list = generic_array_copy(rtobj->event_map_list))){
    return -1;
  }

  if ((generic_array_insert_element(dup_map_list, pos, (void*)(&map))) < 0){
    return -1;
  }

  if ((pointer_hot_swap( (void*)(&rtobj->event_map_list), (void*)(&dup_map_list))) < 0){
    return -1;
  }

  generic_array_destroy(dup_map_list);

  return 0;
}

int rtobject_delete_map(rtobject_t* rtobj, int pos){
  generic_array_t* dup_map_list;
  event_map_t* deleted_map;

  if (!(dup_map_list = generic_array_copy(rtobj->event_map_list))){
    return -1;
  }

  if ((generic_array_remove_element(dup_map_list, pos, (void*)(&deleted_map))) < 0){
    return -1;
  }

  if ((pointer_hot_swap( (void*)(&rtobj->event_map_list), (void*)(&dup_map_list))) < 0){
    return -1;
  }

  generic_array_destroy(dup_map_list);

  /*free deleted map*/
  event_map_dealloca(deleted_map);

  return 0;
}

int rtobject_append_map(rtobject_t* rtobj, event_map_t* map){
  generic_array_t* dup_map_list;
  
  if (!(dup_map_list = generic_array_copy(rtobj->event_map_list))){
    return -1;
  }

  if ((generic_array_append_element(dup_map_list, (void*)(&map))) < 0){
    return -1;
  }

  if ((pointer_hot_swap( (void*)(&rtobj->event_map_list), (void*)(&dup_map_list))) < 0){
    return -1;
  }

  generic_array_destroy(dup_map_list);

  return 0;
}

void rtobject_flush_map_list(rtobject_t* rtobj){
  generic_array_t* dup_map_list;
  int i, map_count;

  map_count = rtobject_get_map_list_size(rtobj);

  /*make empty map list, swap, empty old map list*/

  if (!(dup_map_list = generic_array_create(sizeof(void*)))){
    printf("flush map list error: unable to create empty map list\n");
    return;
  }

  if ((pointer_hot_swap( (void*)(&rtobj->event_map_list), (void*)(&dup_map_list))) < 0){
    printf("unrecoverable error while flushing map list, pointer swap failed\n");
    return;
  }

  /*free deleted maps*/
  for (i=0;i<map_count;++i)
    event_map_dealloca( *(event_map_t**)generic_array_get_element_pointer(dup_map_list, i) );
  
  generic_array_destroy(dup_map_list);

}



/*rtobject map test list fxns: (TODO move these elsewhere maybe?)*/

int rtobject_get_test_list_size(rtobject_t* rtobj, int map_index){
  event_map_t* map;
  if ((rtobject_get_map(rtobj, &map, map_index)) < 0){
    return -1;
  }

  return event_map_get_test_list_size(map);
}


int rtobject_get_test(rtobject_t* rtobj, map_test_t** ptr, int map_index, int pos){
  event_map_t* map;
  if ((rtobject_get_map(rtobj, &map, map_index)) < 0){
    return -1;
  }

  if (!(*ptr = event_map_get_test(map, pos))){
    return -1;
  }

  return 0;
}

int rtobject_insert_test(rtobject_t* rtobj, map_test_t* test, int map_index, int pos){
  event_map_t *map, *dup_map;
  if ((rtobject_get_map(rtobj, &map, map_index)) < 0){
    return -1;
  }

  if (!(dup_map = event_map_copy(map))){
    return -1;
  }

  if ((event_map_insert_test(dup_map, test, pos)) < 0){
    event_map_dealloca(dup_map);
    return -1;
  }

  if ((pointer_hot_swap( generic_array_get_element_pointer(rtobj->event_map_list, map_index),\
			 (void*)(&dup_map))) < 0){
    return -1;
  }
  
  event_map_dealloca(dup_map);

  return 0;
}

int rtobject_delete_test(rtobject_t* rtobj, int map_index, int pos){
  event_map_t *map, *dup_map;
  if ((rtobject_get_map(rtobj, &map, map_index)) < 0){
    return -1;
  }

  if (!(dup_map = event_map_copy(map))){
    return -1;
  }

  if ((event_map_delete_test(dup_map, pos)) < 0){
    event_map_dealloca(dup_map);
    return -1;
  }

  if ((pointer_hot_swap( generic_array_get_element_pointer(rtobj->event_map_list, map_index),\
			 (void*)(&dup_map))) < 0){
    return -1;
  }
  
  event_map_dealloca(dup_map);

  return 0;
}

int rtobject_append_test(rtobject_t* rtobj, map_test_t* test, int map_index){
  event_map_t *map, *dup_map;
  if ((rtobject_get_map(rtobj, &map, map_index)) < 0){
    return -1;
  }
  
  if (!(dup_map = event_map_copy(map))){
    return -1;
  }

  if ((event_map_insert_test(dup_map, test, -1)) < 0){
    event_map_dealloca(dup_map);
    return -1;
  }

  if ((pointer_hot_swap( generic_array_get_element_pointer(rtobj->event_map_list, map_index),\
			 (void*)(&dup_map))) < 0){
    return -1;
  }
  
  event_map_dealloca(dup_map);

return 0;
}

void rtobject_flush_test_list(rtobject_t* rtobj, int map_index){
event_map_t *map, *dup_map;
  if ((rtobject_get_map(rtobj, &map, map_index)) < 0){
    printf("error occured while flushing test list\n");
    return;
  }
  
  if (!(dup_map = event_map_copy(map))){
    printf("error occured while flushing test list\n");
    return;
  }
 
  event_map_clear_test_list(dup_map);
  
  if ((pointer_hot_swap( generic_array_get_element_pointer(rtobj->event_map_list, map_index),\
			 (void*)(&dup_map))) < 0){
    printf("error occured while flushing test list\n");
    return;
  }
  
  event_map_dealloca(dup_map);

  return;
}


/*rtobject map test action list fxns: (TODO move these elsewhere maybe?)*/

int rtobject_get_action_list_size(rtobject_t* rtobj, int map, int test_pos){
  map_test_t* test;

  if ((rtobject_get_test(rtobj, &test, map, test_pos)) < 0){
    printf("get action list size error: couldn't find test\n");
    return -1;
  }

  return map_test_get_action_list_size(test);
}

int rtobject_get_action(rtobject_t* rtobj, map_action_t** ptr, int map, int test_pos, int pos){
  map_test_t* test;

  if ((rtobject_get_test(rtobj, &test, map, test_pos)) < 0){
    printf("get action error: couldn't find test\n");
    return -1;
  }
  
  if (!(*ptr = map_test_get_action(test, pos))){
    printf("get action error: couldn't find action\n");
    return -1;
  }

  return 0;
}

int rtobject_insert_action(rtobject_t* rtobj, map_action_t* action, int map, int test_pos, int pos){
  map_test_t* test;

  if ((rtobject_get_test(rtobj, &test, map, test_pos)) < 0){
    printf("insert action error: couldn't find test\n");
    return -1;
  }

  if (map_test_insert_action(test, action, pos) < 0){
    printf("insert action error: couldn't insert action\n");
    return -1;
  }

  return 0;
}

int rtobject_delete_action(rtobject_t* rtobj, int map, int test_pos, int pos){
  map_test_t* test;

  if ((rtobject_get_test(rtobj, &test, map, test_pos)) < 0){
    printf("delete action error: couldn't find test\n");
    return -1;
  }


  if (map_test_delete_action(test, pos) < 0){
    printf("delete action error: couldn't delete action\n");
    return -1;
  }

  return 0;
}

int rtobject_append_action(rtobject_t* rtobj, map_action_t* action, int map, int test_pos){
  map_test_t* test;

  if ((rtobject_get_test(rtobj, &test, map, test_pos)) < 0){
    printf("append action error: couldn't find test\n");
    return -1;
  }

  if (map_test_insert_action(test, action, -1) < 0){
    printf("append action error: couldn't append action\n");
    return -1;
  }

  return 0;
}

void rtobject_flush_action_list(rtobject_t* rtobj, int map, int test_pos){
  map_test_t* test;

  if ((rtobject_get_test(rtobj, &test, map, test_pos)) < 0){
    printf("flush action list error: couldn't find test\n");
    return;
  }

  map_test_clear_action_list(test);
}












