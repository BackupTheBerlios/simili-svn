#include "list.h"
#include "misc.h"


int
list_remove (list_t * list, node_t * node)
{
  //FIXME:add extra checks

  if (list->first == node)
    {
      if (!node->next)
	{
	  list->first = node->prev;
	}
      else
	{
	  list->first = node->next;
	}
    }

  if (list->last == node)
    {
      if (!node->prev)
	{
	  list->last = node->next;
	}
      else
	{
	  list->last = node->prev;
	}
    }

  if (node->prev)
    {
      node->prev->next = node->next;
    }

  if (node->next)
    {
      node->next->prev = node->prev;
    }

  node->prev = 0;
  node->next = 0;

  return 0;
}


int
list_add_beg (list_t * list, node_t * node)
{
  if(!list)
    return -1;
  node->prev = 0;
  node->next = list->first;
  if (node->next)
    {
      node->next->prev = node;
    }
  list->first = node;
  if (!list->last)
    {
      list->last = node;
    }

  return 0;
}


int
list_add_end (list_t * list, node_t * node)
{
  node->next = 0;
  node->prev = list->last;
  if (node->prev)
    {
      node->prev->next = node;
    }
  list->last = node;
  if (!list->first)
    {
      list->first = node;
    }

  return 0;
}


int
list_insort(list_t *list, node_t *node, int evaluator(void*))
{
   if(!list->cursor)
     list->cursor=list->last;
   if(!list->cursor)
     return list_add_beg(list, node);

//   printf("evaluator(list->cursor->data): %6d, evaluator(node->data): %d\n", evaluator(list->cursor->data), evaluator(node->data));

   if(evaluator(list->cursor->data)<evaluator(node->data))
     {
	for(; list->cursor; list->cursor=list->cursor->next)
	  if(evaluator(list->cursor->data)>evaluator(node->data))
	    return list_insert_before(list, list->cursor, node);
	return list_add_end(list, node);
     }
   else
     {
	for(; list->cursor; list->cursor=list->cursor->prev)
	  if(evaluator(list->cursor->data)<evaluator(node->data))
	    return list_insert_after(list, list->cursor, node);
	return list_add_beg(list, node);
     }
   return -1;
}


int
list_insert_after (list_t * list, node_t * noth, node_t * node)
{
  //  ck_err (!list->first || !list->last);//assert

  if (!noth)
    return list_add_beg (list, node);

  node->next = noth->next;
  noth->next = node;
  node->prev = noth;
  if (list->last == noth)
    {
      list->last = node;
    }
  else
    {
      ck_err (!noth->next);	//assert
      node->next->prev = node;
    }

  return 0;
error:
  return -1;
}


int
list_insert_before (list_t * list, node_t * noth, node_t * node)
{
  //  ck_err (!list->first || !list->last);//assert

  if (!noth)
    return list_add_end (list, node);

  node->prev = noth->prev;
  noth->prev = node;
  node->next = noth;
  if (list->first == noth)
    {
      list->first = node;
    }
  else
    {
      ck_err (!noth->prev);	//assert
      node->prev->next = node;
    }

  return 0;
error:
  return -1;
}


node_t*
node_create(void *data)
{
  node_t *node;
  ck_err(!(node = Talloc(node_t)));
  node->data = data;
  return node;
 error:
  return 0;
}

//#define LSTSZ 6

int
list_sort(list_t *list, int evaluator(void*))
{
  node_t *cursor;
  int change=1;
  //  int table[LSTSZ];
  //  int table_place=LSTSZ;

  while(change)
    {
      change=0;
      for(cursor=list->first; cursor->next; cursor=cursor->next)
	{
	  if(evaluator(cursor->data) > evaluator(cursor->next->data))
	    {
	      void *data;
	      data=cursor->data;
	      cursor->data=cursor->next->data;
	      cursor->next->data=data;
	      change=1;
	    }
	}
    }
  return 0;
}
