#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_

typedef struct list_node {
  void *data;
  struct list_node *next;
} list_node_t;

list_node_t *create_node();
void free_node(list_node_t *node);
void append_node(list_node_t *list, list_node_t *node);

#endif
