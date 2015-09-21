#ifndef HASHMAP_H
#define HASHMAP_H

struct hm;
typedef struct hm hm_t;

hm_t* hm_new(unsigned long initial_capacity, unsigned int (*hash_func)(const void*), int (*key_compare)(const void*,const void*));
void hm_delete(hm_t *hm);
int hm_insert(hm_t *hm, void* data);
void hm_delete_elems(hm_t* hm, void (*elem_free)(void *));
void hm_foreach(hm_t* hm, void (*func)(void *,void*), void* user_data);
void hm_foreach_matching(hm_t* hm,void* data, void (*func)(void *,void*),void* user_data);
int hm_delete_matching(hm_t* hm, void* data);
void* hm_find_one(hm_t* hm, void* data);
#endif
