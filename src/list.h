#ifndef _LIST_H
#define _LIST_H


#define API


typedef struct node_s node_t;
typedef struct list_s list_t;


struct node_s
{
  node_t *prev;
  node_t *next;
  void *data;
};


struct list_s
{
  node_t *first;
  node_t *last;
  node_t *cursor;
  
};


int API list_remove (list_t *, node_t *);
int API list_add_beg (list_t *, node_t *);
int API list_add_end (list_t *, node_t *);
int API list_insert_after (list_t *, node_t *, node_t *);
int API list_insert_before (list_t *, node_t *, node_t *);
node_t* API node_create(void*);
int API list_insort(list_t *list, node_t *node, int evaluator(void*));


#endif
