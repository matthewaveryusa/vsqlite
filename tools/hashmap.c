#include <tools/hashmap.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct hm
{
  //array of void pointer arrays
  void*** _buckets;
  unsigned int* _bucket_lengths;
  unsigned long _elems;
  unsigned long _capacity;
  unsigned int (*_hash_func)(const void *);
  int (*_key_compare)(const void *, const void *);
};

static int hm_rebucket(hm_t* hm);

hm_t* hm_new(unsigned long initial_capacity, unsigned int (*hash_func)(const void*), int (*key_compare)(const void*,const void*)) {
  assert(hash_func != NULL);
  assert(key_compare != NULL);
  hm_t* hm = (hm_t*) malloc(sizeof(hm_t));
  if(hm == NULL) { return NULL;}
  hm->_elems = 0;
  hm->_hash_func = hash_func;
  hm->_key_compare = key_compare;
  hm->_capacity = initial_capacity * 4 / 3;
  hm->_buckets = malloc(initial_capacity * sizeof(void ***));
  if(hm->_buckets == NULL) {
    free(hm);
    return NULL;
  }
  hm->_bucket_lengths = malloc(initial_capacity * sizeof(unsigned int));
  if(hm->_buckets == NULL) {
    free(hm->_buckets);
    free(hm);
    return NULL;
  }
  memset(hm->_bucket_lengths, 0, initial_capacity * sizeof(unsigned int));
  return hm;
}

void hm_delete(hm_t *hm) {
  free(hm->_buckets);
  free(hm->_bucket_lengths);
  free(hm);
}

int hm_insert(hm_t *hm, void* data) {
  void **tmp;
  int ret;
  unsigned int hash;
  ret = hm_rebucket(hm);
  if(ret != 0) {
    return ret;
  }
  hash = hm->_hash_func(data) % hm->_capacity;
  tmp = realloc(hm->_buckets[hash],(hm->_bucket_lengths[hash]+1)* sizeof(void**));
  if(tmp == NULL) {
    return -1;
  }
  ++hm->_bucket_lengths[hash];
  tmp[hm->_bucket_lengths[hash] - 1] = data;
  hm->_buckets[hash] = tmp;
  return 0;
}

void hm_delete_elems(hm_t* hm, void (*elem_free)(void *)) {
  int i,j;
  for(i = 0; i < hm->_elems; ++i) {
    if(hm->_bucket_lengths[i] == 0) continue;
    for(j = 0; j < hm->_bucket_lengths[i]; ++j) {
      elem_free(hm->_buckets[i][j]);
    }
  }
}

void hm_foreach(hm_t* hm, void (*func)(void *,void*), void* user_data) {
  int i,j;
  for(i = 0; i < hm->_elems; ++i) {
    if(hm->_bucket_lengths[i] == 0) continue;
    for(j = 0; j < hm->_bucket_lengths[i]; ++j) {
      func(hm->_buckets[i][j],user_data);
    }
  }
}

void hm_foreach_matching(hm_t* hm,void* data, void (*func)(void *,void*), void* user_data) {
  int j;
  unsigned int hash = hm->_hash_func(data) % hm->_capacity;
  if(hm->_bucket_lengths[hash] == 0) return;
  for(j = 0; j < hm->_bucket_lengths[hash]; ++j) {
    if( hm->_key_compare(data,hm->_buckets[hash][j]) == 0) {
      func(hm->_buckets[hash][j],user_data);
    }
  }
}

int hm_delete_matching(hm_t* hm, void* data) {
  int j,deleted = 0;
  unsigned int hash;
  void **b,**tmp;
  hash = hm->_hash_func(data) % hm->_capacity;
  b = hm->_buckets[hash];
  unsigned int* blen = &(hm->_bucket_lengths[hash]);
  if(*blen == 0) return 0;
  for(j = 0; j < *blen; ++j) {
    if( hm->_key_compare(data,b[j]) == 0) {
      ++deleted;
      --(*blen);
      b[j] = b[*blen];
    }
  }
  tmp = realloc(b,*blen * sizeof(void **));
  if(tmp != NULL) {
    hm->_buckets[hash] = tmp;
  }
  return deleted;
}

void* hm_find_one(hm_t* hm, void* data) {
  int j;
  unsigned int hash = hm->_hash_func(data) % hm->_capacity;
  if(hm->_bucket_lengths[hash] == 0) return NULL;
  for(j = 0; j < hm->_bucket_lengths[hash]; ++j) {
    if( hm->_key_compare(data,hm->_buckets[hash][j]) == 0) {
      return hm->_buckets[hash][j];
    }
  }
  return NULL;
}

/*rebucket. Not done in-place so that an allocation error doesn't put the hashmap
in an unknown state.*/
static int hm_rebucket(hm_t* hm) {
  int i, j;
  void ***tmp_buckets;
  unsigned int hash, *tmp_bucket_lengths;

  if((hm->_capacity * 3 +2 )/ 4 > hm->_elems) {
    return 0;
  }
  unsigned int new_capacity = hm->_capacity*2;
  tmp_buckets = malloc(new_capacity * sizeof(void***));
  if(tmp_buckets == NULL) { return -1;}
  tmp_bucket_lengths = malloc(new_capacity * sizeof(unsigned int*));
  if(tmp_bucket_lengths == NULL) { 
    free(tmp_buckets);
    return -1;
  }
  memset(tmp_bucket_lengths, 0, new_capacity * sizeof(void***));
  memset(tmp_buckets, 0, new_capacity * sizeof(unsigned int));
  for(i = 0; i < hm->_elems; ++i) {
    if(hm->_bucket_lengths[i] == 0) continue;
    for(j = 0; j < hm->_bucket_lengths[i]; ++j) {
      hash = hm->_hash_func(hm->_buckets[i][j]) % new_capacity;
      ++tmp_bucket_lengths[hash];
      tmp_buckets[hash] = realloc(tmp_buckets[hash],sizeof(void**)*tmp_bucket_lengths[hash]);
      if(tmp_buckets[hash] == NULL) {
        free(tmp_buckets);
        free(tmp_bucket_lengths);
        return -1;
      }
      tmp_buckets[hash][tmp_bucket_lengths[hash]-1] = hm->_buckets[i][j];
    }
  }
  free(hm->_buckets);
  free(hm->_bucket_lengths);
  hm->_capacity = new_capacity;
  hm->_buckets = tmp_buckets;
  hm->_bucket_lengths = tmp_bucket_lengths;
  return 0;
}
