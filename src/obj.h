#ifndef _OBJ_H_
#define _OBJ_H_


//Who cares ?
#include "simili.h"


//FUNCTIONS
obj_c* obj_class();
obj_t* check_ancestor(obj_t*, obj_c*, char*, int);
obj_t* _obj_create(int, obj_c*);

//MACROS
//CAST
#define OBJ_CLASS(class) ((obj_c*)(class))
#ifdef CHECK_CAST
#define OBJ(obj) ((obj_t*)check_ancestor((obj_t*)(obj), obj_class(), __FILE__, __LINE__))
#else
#define OBJ(obj) ((obj_t*)(obj))
#endif
//TEST
#define IS_OBJ(obj) (OBJ(obj)->class == obj_class())
//FUNC
#define obj_create(type) ((type##_t*)_obj_create(sizeof(type##_t), type##_class()))


#define obj_name(obj) (OBJ(obj)->class->name)


#define create_class(cla, anc) \
      static cla##_c static_class; \
      class = OBJ_CLASS(&static_class); \
      *(anc##_c*)class = *(anc##_c*)anc##_class(); \
      class->ancestor = anc##_class(); \
      class->name = #cla;

 
#endif
