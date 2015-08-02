#ifndef VECTOR_H
#define VECTOR_H

void* vec_get(void* v, unsigned long index);
void* vec_new(unsigned long elemsize, unsigned long initial_capacity);
void vec_delete_elems(void* v, void (*elem_free)(void *));
void vec_foreach(void* v, void (*func)(void *));
void vec_delete(void* v);
void vec_grow(void* v);
void vec_push_back(void* v, void* data, unsigned long elemsize);
void* vec_push_back_uninitialized(void* v, unsigned long elemsize);
unsigned long vec_length(void* v);
void vec_copy_item(void* v, void* dest, unsigned long index);
void* vec_move_and_delete(void *v);
#endif
