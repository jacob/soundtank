/*Copyright 2003 Jacob Robbins*/

#include <stdio.h>
#include <stdlib.h>

#include "linked_list.h"


ll_head list;
ll_head other_list;

void print_list(){
  printf("\nlist= ");
  if (list){
    node_t* temp_node;
    for(temp_node = list;temp_node;temp_node=temp_node->next){
      printf("%d, ",*((int*)temp_node->data));

    }
    /*
    printf("\nlist backwards= ");
    for(temp_node = list;temp_node->next;temp_node=temp_node->next) ;
    while (temp_node){
      printf("%d, ",*((int*)temp_node->data));
      temp_node=temp_node->previous;
    }
    */
  }else printf("empty list");
  printf("\n");
}

void print_both_lists(){
  print_list();
  printf("\nother list= ");
  if (other_list){
    node_t* temp_node;
    for(temp_node = other_list;temp_node;temp_node=temp_node->next){
      printf("%d, ",*((int*)temp_node->data));

    }
    /*
    printf("\nother list backwards= ");
    for(temp_node = other_list;temp_node->next;temp_node=temp_node->next) ;
    while (temp_node){
      printf("%d, ",*((int*)temp_node->data));
      temp_node=temp_node->previous;
    }
    */
  }else printf("empty list");
  printf("\n");
} 



int main(int argc, char** args){
  node_t* temp;
  node_t* temp2;
  int* data;
  int i;
  list = 0;
  other_list =0;

  printf("\nNodetest:\n");
  print_list();

  printf("\nprepend 1,2,3\n");
  data = (int*)malloc(sizeof(int));
  *data = 1;
  temp = ll_prepend(&list,data);
  data = (int*)malloc(sizeof(int));
  *data = 2;
  temp = ll_prepend(&list,data);
  data = (int*)malloc(sizeof(int));
  *data = 3;
  temp = ll_prepend(&list,data);
  print_list();

  printf("\nappend 1,2,3,9,10,11,12\n");
  data = (int*)malloc(sizeof(int));
  *data = 1;
  temp = ll_append(&list,data);
  data = (int*)malloc(sizeof(int));
  *data = 2;
  temp = ll_append(&list,data);
  data = (int*)malloc(sizeof(int));
  *data = 3;
  temp = ll_append(&list,data);
   data = (int*)malloc(sizeof(int));
  *data = 9;
  temp = ll_append(&list,data);
  data = (int*)malloc(sizeof(int));
  *data = 10;
  temp = ll_append(&list,data);
  data = (int*)malloc(sizeof(int));
  *data = 11;
  temp = ll_append(&list,data);
  data = (int*)malloc(sizeof(int));
  *data = 12;
  temp = ll_append(&list,data);
 print_list();


  printf("\ninsert_after 5-first element 6-last element\n");
  data = (int*)malloc(sizeof(int));
  *data = 5;
  temp = ll_insert_after(list,data);
  for(temp=list;temp->next;temp=temp->next) ;
  data = (int*)malloc(sizeof(int));
  *data = 6;
  temp = ll_insert_after(temp,data);
  print_list();


  printf("\ninsert_before 7-first element 8-fourth element\n");
  data = (int*)malloc(sizeof(int));
  *data = 7;
  temp = ll_insert_before(list,data,&list);
  temp=list;
  for(i=0;i<3;++i) temp=temp->next;
  data = (int*)malloc(sizeof(int));
  *data = 8;
  temp = ll_insert_before(temp,data,&list);
  print_list();

  printf("\n     .....Inter-list operations:\n");

  printf("\ncut 2 first elements from list and paste prepend to other list\n");
  temp = ll_cut(list,&list);
  ll_paste_prepend(temp,&other_list);
  temp = ll_cut(list,&list);
  ll_paste_prepend(temp,&other_list);
  print_both_lists();

  printf("\ncut 2 first elements from list and paste append to other list\n");
  temp = ll_cut(list,&list);
  ll_paste_append(temp,&other_list);
  temp = ll_cut(list,&list);
  ll_paste_append(temp,&other_list);
  print_both_lists();
    
  printf("\ncut first element from list and paste after first element in other list\n");
  temp = ll_cut(list,&list);
  ll_paste_after(other_list,temp);
  print_both_lists();
  
  printf("\ncut first element from list and paste after last element in other list\n");
  temp = ll_cut(list,&list);
  for(temp2=other_list;temp2->next;temp2=temp2->next) ;
  ll_paste_after(temp2,temp);
  print_both_lists();
  
  printf("\ncut first element from list and paste before first element in other list\n");
  temp = ll_cut(list,&list);
  ll_paste_before(other_list,temp,&other_list);
  print_both_lists();

  printf("\ncut first element from list and paste after last element in other list\n");
  temp = ll_cut(list,&list);
  for(temp2=other_list;temp2->next;temp2=temp2->next) ;
  ll_paste_after(temp2,temp);
  print_both_lists();
   
  printf("\ncut first element from list and paste before last element in other list\n");
  temp = ll_cut(list,&list);
  for(temp2=other_list;temp2->next;temp2=temp2->next) ;
  ll_paste_before(temp2,temp,&other_list);
  print_both_lists();
  
  printf("\n     .....Section-sized inter-list operations:\n");

  printf("\ncut last 3 elements from other list and paste prepend to list\n");
  for(temp2=other_list;temp2->next;temp2=temp2->next) ;
  for (i=0;i<2;++i) temp2=temp2->previous;
  temp = ll_section_cut(temp2,3,&other_list);
  ll_section_paste_prepend(temp,&list);
  print_both_lists();

  printf("\ncut last 3 elements from other list and paste append to list\n");
  for(temp2=other_list;temp2->next;temp2=temp2->next) ;
  for (i=0;i<2;++i) temp2=temp2->previous;
  temp = ll_section_cut(temp2,6,&other_list);
  ll_section_paste_append(temp,&list);
  print_both_lists();

  printf("\ncut 2nd through 4th elements from list and paste after first element in other list\n");
  temp2=list->next;
  temp = ll_section_cut(temp2,3,&list);
  ll_section_paste_after(other_list,temp);
  print_both_lists();

  printf("\ncut first 3 elements from list and paste before first element in other list\n");
  temp2=list;
  temp = ll_section_cut(temp2,3,&list);
  ll_section_paste_before(other_list,temp,&other_list);
  print_both_lists();

  printf("\ncut first 3 elements from other list and paste before last element in list\n");
  temp2=other_list;
  temp = ll_section_cut(temp2,3,&other_list);
  for(temp2=list;temp2->next;temp2=temp2->next) ;
  ll_section_paste_before(temp2,temp,&list);
  print_both_lists();

  printf("\ncut first 3 elements from other list and paste after last element in list\n");
  temp2=other_list;
  temp = ll_section_cut(temp2,3,&other_list);
  for(temp2=list;temp2->next;temp2=temp2->next) ;
  ll_section_paste_after(temp2,temp);
  print_both_lists();





  printf("\nremove first element and last element from list\n");
  ll_remove(list,&list);
  for(temp=list;temp->next;temp=temp->next) ;
  ll_remove(temp,&list);
  print_list();

  printf("\nremove all elements\n");
  while(list) ll_remove(list,&list);
  print_list();

  
  return 0;
}
