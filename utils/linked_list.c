#include "linked_list.h"
#include <stdlib.h>

list_node_t *create_node() {
  list_node_t *list = (list_node_t *)calloc(1, sizeof(list_node_t));
  return list;
}

void free_node(list_node_t *node) {
  list_node_t *next;
  while (node) {
    next = node->next;
    free(node);
    node = next;
  }
}

void append_node(list_node_t *list, list_node_t *node) {
  list_node_t *curr = list;
  while (curr->next) {
    curr = curr->next;
  }
  curr->next = node;
}
