/*Copyright 2003 Jacob Robbins*/

#ifndef UTIL_LINKED_LIST_INCLUDES
#define UTIL_LINKED_LIST_INCLUDES

typedef struct ll_node node_t;
typedef node_t* ll_head;

struct ll_node{
  struct ll_node* next;
  struct ll_node* previous;
  void* data;
};


int ll_get_size(const ll_head* head);
node_t* ll_get_node(const ll_head* head, int nodepos);
node_t* ll_get_last_node(const ll_head* head);

int ll_copy(ll_head* dest, const ll_head *src);
void ll_free_all(ll_head* head);

node_t* ll_prepend(ll_head* head, void* data);
node_t* ll_append(ll_head* head, void* data);
node_t* ll_insert_after(node_t* node, void* data);
node_t* ll_insert_before(node_t* node, void* data, ll_head* head);
void ll_remove(node_t* node, ll_head* head);

/*the following cut/paste fxns don't allocate memory so are realtime safe*/
void ll_paste_prepend(node_t* freenode, ll_head* head);
void ll_paste_append(node_t* freenode, ll_head* head);
void ll_paste_after(node_t* targetnode, node_t* freenode);
void ll_paste_before(node_t* targetnode, node_t* freenode, ll_head* head);
node_t* ll_cut(node_t* node, ll_head* head);

void ll_section_paste_prepend(node_t* paste_list, ll_head* head);
void ll_section_paste_append(node_t* paste_list, ll_head* head);
void ll_section_paste_after(node_t* targetnode, node_t* paste_list);
void ll_section_paste_before(node_t* targetnode, node_t* paste_list, ll_head* head);
node_t* ll_section_cut(node_t* node, int section_size, ll_head* head);



#endif
