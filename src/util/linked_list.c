/*Copyright 2003 Jacob Robbins*/

#include <stdlib.h>

#include "../include.h"


/*typedef struct ll_node node_t;*/
/*typedef node_t* ll_head;*/

/*
struct ll_node{
  struct ll_node* next;
  struct ll_node* previous;
  void* data;
}
*/

int ll_get_size(const ll_head* head){
  int i;
  node_t* curr;
  if (!(*head)) return 0;
  
  i=0;
  for (curr = *head; curr; curr=curr->next) ++i;
  
  return i;
}

node_t* ll_get_node(const ll_head* head, int nodepos){
  node_t* curr;
  int i;
  if (!(*head)) return 0;
  if (nodepos < 0) return 0;
  
  curr = *head;
  for (i=0;i<nodepos;++i){
    curr=curr->next;
    if (!curr) return 0;
  }
 
  return curr;
}

node_t* ll_get_last_node(const ll_head* head){
  node_t* curr;

  if (!(curr = *head))
    return 0;

  while (curr->next) curr=curr->next;

  return curr;
}


int ll_copy(ll_head* dest, const ll_head *src){
  node_t *curr;

  for (curr=*src;curr;curr=curr->next){
    
    if (!(ll_append(dest, curr->data))){
      return -1;
    }
    
  }

  return 0;
}

void ll_free_all(ll_head* head){
  node_t *curr, *next;

  /*walk list backwards*/
  next = *head;

  while (next){
    
    curr = next;
    next = next->next;

    free(curr);

  }

}

node_t* ll_prepend(ll_head* head, void* data){

  node_t* new_node = (node_t*)malloc(sizeof(node_t));
  if (!new_node) return new_node;

  new_node->data = data;
  new_node->previous = 0;

  if (!(*head)){
    (*head) = new_node;
    new_node->next = 0;
  }else{
    new_node->next = (*head);
    new_node->next->previous = new_node;
    (*head) = new_node;
  }
  return new_node;
}

node_t* ll_append(ll_head* head, void* data){

  node_t* new_node = (node_t*)malloc(sizeof(node_t));
  if (!new_node) return new_node;

  new_node->data = data;
  new_node->next = 0;

  if (!(*head)){

    (*head) = new_node;
    new_node->previous = 0;

  }else{

    node_t* temp_node;  
    for(temp_node=(*head);temp_node->next;temp_node=temp_node->next) ; 
    temp_node->next = new_node;
    new_node->previous = temp_node;

  }

  return new_node;
}

node_t* ll_insert_after(node_t* node, void* data){

  node_t* new_node = (node_t*)malloc(sizeof(node_t));
  if (!new_node) return new_node;

   new_node->data = data;
   new_node->previous = node;

   if (!node->next){
 
     node->next = new_node;
     new_node->next = 0;

   }else{
     
     new_node->next = node->next;
     new_node->next->previous = new_node;
     node->next = new_node;

   }
   
   return new_node;

}

node_t* ll_insert_before(node_t* node, void* data, ll_head* head){

  if (!node->previous){

    return ll_prepend(head,data);

  }else{
  
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    if (!new_node) return new_node;
    
    new_node->data = data;
    new_node->next = node;
    new_node->previous = node->previous;
    node->previous = new_node;
    new_node->previous->next = new_node;
    
    return new_node;
  }

}

void ll_remove(node_t* node, ll_head* head){

  if (!node->previous){

    if (!node->next){

      (*head) = 0;

    }else{

      node->next->previous =0;
      (*head) = node->next;

    }

  }else{

    if (!node->next){

      node->previous->next = 0;

    }else{

      node->next->previous = node->previous;
      node->previous->next = node->next;

    }
  }
 
  free(node);

}





void ll_paste_prepend(node_t* freenode, ll_head* head){
  freenode->previous = 0;
  if (*head){
    freenode->next = (*head);
    freenode->next->previous = freenode;
  }else{
    freenode->next = 0;
  }
  (*head) = freenode;
}


void ll_paste_append(node_t* freenode, ll_head* head){
  node_t* temp_node;

  if (*head){
    for (temp_node=(*head);temp_node->next;temp_node=temp_node->next) ;
    freenode->previous = temp_node;
    freenode->previous->next = freenode;
  }else{
    (*head) = freenode;
  }  
  freenode->next = 0;
}


void ll_paste_after(node_t* targetnode, node_t* freenode){

  freenode->previous = targetnode;

  if (targetnode->next){
    freenode->next = targetnode->next;
    freenode->next->previous = freenode;

  }else{
    freenode->next = 0;
  }
  targetnode->next = freenode;
}


void ll_paste_before(node_t* targetnode, node_t* freenode, ll_head* head){

  freenode->next = targetnode;
  
  if (!targetnode->previous){
    (*head) = freenode;
    targetnode->previous = freenode;

  }else{
    freenode->previous = targetnode->previous;
    freenode->previous->next = freenode;
    targetnode->previous = freenode;
  }
}


node_t* ll_cut(node_t* node, ll_head* head){

  if (!node->previous){
    if (!node->next){
      (*head)=0;
    }else{
      (*head)=node->next;
      node->next->previous = 0;
    }
  }else{
    if (!node->next){
      node->previous->next = 0;
    }else{
      node->previous->next = node->next;
      node->next->previous = node->previous;
    }
  }
  
  node->next = node->previous = 0; /*make sure returned node is dereferenced*/

  return node;
}








void ll_section_paste_prepend(node_t* paste_list, ll_head* head){
  node_t* temp_node;
  
  if (!(*head)){
    (*head) = paste_list;
  }else{
    for (temp_node=paste_list;temp_node->next;temp_node=temp_node->next) ;
    temp_node->next = *head;
    (*head)->previous = temp_node;
    (*head) = paste_list;
  }

}

void ll_section_paste_append(node_t* paste_list, ll_head* head){
  node_t* temp_node;
  
  if (!(*head)){
    (*head) = paste_list;
  }else{
    for (temp_node=(*head);temp_node->next;temp_node=temp_node->next) ;
    paste_list->previous = temp_node;
    temp_node->next = paste_list;
  }

}

void ll_section_paste_after(node_t* targetnode, node_t* paste_list){
  node_t* copyfrom_node;
  node_t* copyto_node;
  node_t* curr_node;

  copyfrom_node = paste_list;
  copyto_node = targetnode;
  while (copyfrom_node){
    curr_node = copyfrom_node;
    copyfrom_node=copyfrom_node->next;		   
    ll_paste_after(copyto_node,curr_node);
    copyto_node = curr_node;
  }
}


void ll_section_paste_before(node_t* targetnode, node_t* paste_list, ll_head* head){
  node_t* temp_node;
  node_t* other_node;

  temp_node = paste_list;
  while(temp_node){
    other_node = temp_node;
    temp_node=temp_node->next;
    ll_paste_before(targetnode,other_node,head);
  }


}

node_t* ll_section_cut(node_t* node, int section_size, ll_head* head){
  node_t* cut_list;
  node_t* curr_node;
  node_t* copyfrom_node;
  node_t* copyto_node;
  int i;

  copyfrom_node = node->next;
  cut_list = ll_cut(node,head);
  cut_list->next = 0;
  cut_list->previous = 0; /*make sure returned section is dereferenced*/
  copyto_node = cut_list;
  
  for (i=1;i<section_size;++i){
    if (!copyfrom_node) return cut_list;
    curr_node=copyfrom_node;
    copyfrom_node=copyfrom_node->next;
    ll_paste_after(copyto_node,ll_cut(curr_node,head));
    copyto_node = curr_node;
  }

  return cut_list;
}


