#ifndef LIST_H
#define LIST_H

struct node {
   int fd;
   struct node *next;
};

extern struct node *head;
extern struct node *current;


void insertFirst(int fd);
struct node* delete(int key);
void deleteList();
void printList();

#endif
