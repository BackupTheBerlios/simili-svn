#include <stdio.h>
#include <stdlib.h>
#include "misc.h"
#include "obj.h"


int
destroy(obj_t *obj)
{
  printf("obj <<%s>> destroyed\n", obj_name(obj));
  free(obj);
  return 0;
}


obj_c*
obj_class()
{
  static obj_c *class=0;

  if(!class)
    {
      static obj_c static_class;

      static_class.ancestor = 0;
      static_class.destroy = destroy;
      static_class.name = "obj";
      class = &static_class;
    }

  return class;
}


obj_t*
_obj_create(int size, obj_c *class)
{
  obj_t *obj=0;  

  ck_err(!(obj = (obj_t*)calloc(size, 1)));
  obj->class = class;

  return obj;
 error:
  return 0;
}


int
print_ancestor(obj_c *class)
{
  int i, deep;

  if(!class)
    return 0;

  deep = print_ancestor(class->ancestor);

  for(i=0; i<deep; i++)
    {
      printf("   ");
    }

  printf("%s\n", class->name);

  return deep + 1;
}


obj_t*
check_ancestor(obj_t *obj, obj_c *ancestor_class, char *file, int line)
{
  obj_c *class;

  if(!obj->class)
    return obj;

  for(class=obj->class; class; class = class->ancestor)
    {
      if(ancestor_class == class)
	return obj;
    }

  if(file)
    {
      printf("Hé coco ! ton obj dans %s[%d], il hérite pas de %s, c'est un %s\n", file, line, ancestor_class->name, obj_name(obj));
      printf("jettes un coup d'oeil à cet arbre et ne recommences plus ! ;p\n---début de l'arbre---\n");
      print_ancestor(obj->class);
      printf("---fin de l'arbre---\n");
    }
  // error:
  return 0;
}

