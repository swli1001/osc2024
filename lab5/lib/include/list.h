#ifndef LIST_H
#define LIST_H

#include <stdint.h>


struct list {
  struct listItem* first;
  struct listItem* last;
  uint32_t itemCount;
};

struct listItem {
  struct listItem* prev;
  struct listItem* next;
  uint32_t size;
  void *data;
};

void listAppend(struct list *lst, struct listItem *item);
void listRemoveItem(struct list *lst, struct listItem *item);

#endif