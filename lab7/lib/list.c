// #include <stddef.h>
#include "list.h"
#define NULL 0

void listAppend(struct list *lst, struct listItem *item) {
  if (lst->itemCount == 0) {
    lst->itemCount = 1;
    lst->first = item;
    lst->last = item;
    item->prev = NULL;
    item->next = NULL;
  } else {
    lst->itemCount += 1;
    item->prev = lst->last;
    item->next = NULL;
    lst->last->next = item;
    lst->last = item;
  }
}

void listRemoveItem(struct list *lst, struct listItem *item) {
  if (lst->itemCount == 1) {
    lst->last = NULL;
    lst->first = NULL;
    lst->itemCount = 0;
  } else {
    if (item == lst->last) {
      lst->last = lst->last->prev;
    }
    if (item == lst->first) {
      lst->first = lst->first->next;
    }
    lst->itemCount--;
    if (item->prev != NULL) {
      item->prev->next = item->next;
    }
    if (item->next != NULL) {
      item->next->prev = item->prev;
    }
    item->prev = NULL;
    item->next = NULL;
  }
}