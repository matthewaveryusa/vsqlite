/*vector.c*/

#include <tools/vector.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct vec
{
  unsigned char* _mem;
  unsigned long _elems;
  unsigned long _elemsize;
  unsigned long _capacity;
};

void* vec_get(vec_t* pvec, unsigned long index)
{
  assert(index < pvec->_elems);
  return (void*)(pvec->_mem + (index * pvec->_elemsize));
}


vec_t* vec_new(unsigned long elemsize, unsigned long initial_capacity)
{
  vec_t* pvec = (vec_t*)malloc(sizeof(vec_t));
  pvec->_capacity = initial_capacity;
  pvec->_elemsize = elemsize;
  pvec->_elems = 0;
  pvec->_mem = (unsigned char*)malloc(pvec->_capacity * pvec->_elemsize);
  return pvec;
}

void vec_delete_elems(vec_t* pvec, void (*elem_free)(void *))
{
  int i;
  for(i = 0; i < pvec->_elems; ++i) {
    elem_free(vec_get(pvec,i));
  }
}

void vec_foreach(vec_t* pvec, void (*func)(void *elem, void* user_data), void* user_data)
{
  int i;
  for(i = 0; i < pvec->_elems; ++i) {
    func(vec_get(pvec,i),user_data);
  }
}

void vec_for_item(vec_t* pvec,unsigned int index, void (*func)(void *elem, void* user_data),void* user_data)
{
    func(vec_get(pvec,index),user_data);
}

void vec_delete(vec_t* pvec)
{
  free(pvec->_mem);
  free(pvec);
}

void* vec_move_and_delete(vec_t* pvec) {
 void* ret = realloc(pvec->_mem,pvec->_elemsize * pvec->_elems);
 pvec->_mem = 0;
 vec_delete(pvec);
 return ret;
}


void vec_grow(vec_t* pvec)
{
  pvec->_mem = (unsigned char*)realloc(pvec->_mem,(pvec->_capacity * 2) * pvec->_elemsize);
  pvec->_capacity *=2;
}

void vec_push_back(vec_t* pvec, void* data, unsigned long elemsize)
{
  assert(elemsize == pvec->_elemsize);
  memcpy(vec_push_back_uninitialized(pvec,elemsize), (unsigned char*)data, elemsize);
}

void* vec_push_back_uninitialized(vec_t* pvec, unsigned long elemsize)
{
  assert(elemsize == pvec->_elemsize);
  if (pvec->_elems == pvec->_capacity) {
    vec_grow(pvec);
  }
  pvec->_elems++;    
  return pvec->_mem + ((pvec->_elems-1) * pvec->_elemsize);
}



unsigned long vec_length(vec_t* pvec)
{
  return pvec->_elems;
}


void vec_copy_item(vec_t* pvec, void* dest, unsigned long index)
{
  memcpy(dest, vec_get(pvec, index), pvec->_elemsize);
}
