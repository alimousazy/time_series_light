#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma once

struct circular_cache {
  char *key;
  void *value;
};



typedef void circular_cache_cleanup_cb(struct circular_cache *item);
struct circular_cache *circular_cache_find(struct circular_cache *list, char *key, int length);
int circular_cache_add(struct circular_cache *list, char *key, void *value, int length);
void circular_cache_cleanup(struct circular_cache *list, int length, circular_cache_cleanup_cb func) ;
