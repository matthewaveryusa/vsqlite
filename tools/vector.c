// vector.c

#include <tools/vector.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct vec
{
  unsigned char* _mem;
  unsigned long _elems;
  unsigned long _elemsize;
  unsigned long _capelems;
  unsigned long _reserve;
} vec_t;

void* vec_get(void* v, unsigned long index)
{
  vec_t* pvec = v;
  assert(index < pvec->_elems);
  return (void*)(pvec->_mem + (index * pvec->_elemsize));
}


void* vec_new(unsigned long elemsize, unsigned long initial_capacity)
{
  vec_t* pvec = (vec_t*)malloc(sizeof(vec_t));
  pvec->_reserve = initial_capacity;
  pvec->_capelems = initial_capacity;
  pvec->_elemsize = elemsize;
  pvec->_elems = 0;
  pvec->_mem = (unsigned char*)malloc(pvec->_capelems * pvec->_elemsize);
  return pvec;
}

void vec_delete_elems(void* v, void (*elem_free)(void *))
{
  vec_t* pvec = v;
  int i;
  for(i = 0; i < pvec->_elems; ++i) {
    elem_free(vec_get(pvec,i));
  }
}

void vec_foreach(void* v, void (*func)(void *))
{
  vec_t* pvec = v;
  int i;
  for(i = 0; i < pvec->_elems; ++i) {
    func(vec_get(pvec,i));
  }
}

void vec_delete(void* v)
{
  vec_t* pvec = v;
  free(pvec->_mem);
  free(pvec);
}

void* vec_move_and_delete(void* v) {
 vec_t* pvec = v;
 void* ret = realloc(pvec->_mem,pvec->_elemsize * pvec->_elems);
 pvec->_mem = 0;
 vec_delete(pvec);
 return ret;
}


void vec_grow(void* v)
{
  vec_t* pvec = v;
  unsigned char* mem = (unsigned char*)malloc((pvec->_capelems + pvec->_reserve) * pvec->_elemsize);
  memcpy(mem, pvec->_mem, pvec->_elems * pvec->_elemsize);
  free(pvec->_mem);
  pvec->_mem = mem;
  pvec->_capelems += pvec->_reserve;
}

void vec_push_back(void* v, void* data, unsigned long elemsize)
{
  vec_t* pvec = v;
  assert(elemsize == pvec->_elemsize);
  memcpy(vec_push_back_uninitialized(v,elemsize), (unsigned char*)data, elemsize);
}

void* vec_push_back_uninitialized(void* v, unsigned long elemsize)
{
  vec_t* pvec = v;
  assert(elemsize == pvec->_elemsize);
  if (pvec->_elems == pvec->_capelems) {
    vec_grow(pvec);
  }
  pvec->_elems++;    
  return pvec->_mem + ((pvec->_elems-1) * pvec->_elemsize);
}



unsigned long vec_length(void* v)
{
  vec_t* pvec = v;
  return pvec->_elems;
}


void vec_copy_item(void* v, void* dest, unsigned long index)
{
  vec_t* pvec = v;
  memcpy(dest, vec_get(pvec, index), pvec->_elemsize);
}
