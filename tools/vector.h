#ifndef VECTOR_H
#define VECTOR_H

struct vec;
typedef struct vec vec_t;

void* vec_get(vec_t* v, unsigned long index);
void vec_for_item(vec_t* pvec,unsigned int index, void (*func)(void *,void*), void* user_data);
vec_t* vec_new(unsigned long elemsize, unsigned long initial_capacity);
void vec_delete_elems(vec_t* v, void (*elem_free)(void *));
void vec_foreach(vec_t* v, void (*func)(void *,void*), void* user_data);
void vec_delete(vec_t* v);
void vec_grow(vec_t* v);
void vec_push_back(vec_t* v, void* data, unsigned long elemsize);
void* vec_push_back_uninitialized(vec_t* v, unsigned long elemsize);
unsigned long vec_length(vec_t* v);
void vec_copy_item(vec_t* v, void* dest, unsigned long index);
void* vec_move_and_delete(vec_t *v);
#endif
