#pragma once

#include "time_convert.h"
#include <rocksdb/c.h>
#include <unistd.h> 
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <libmill.h>

#define METRIC_FORMAT  "%s-%ld"
#define DB_NAME "time_series"
#define SHARD_SIZE 604800
#define SHARD_LEN(x) (sizeof(x) * 604800)
#define DS_MMAP 1
#define DS_MALLOC 2

struct mill_file {
    int fd;
};

struct data_store {
	rocksdb_t *db;
	rocksdb_options_t *options;
	rocksdb_writeoptions_t *writeoptions;
	rocksdb_readoptions_t *readoptions;
  struct circular_cache *circ_cache;
};

struct metric { 
  char *name;
  time_t time;
  float value;
};

struct range_query_result {
  float *points;
  int num_shards;
  time_t shard_size;
  time_t start_date;
  int s_type; 
};

//public method
//
struct range_query_result ds_current(struct data_store *dp, char *metric_name, time_t t_time,  int *error);
time_t init_ds_iter(time_t p_time);
time_t incr_ds_iter(time_t p_time, time_t amount);
struct data_store *create_data_store();
int store_dp(struct data_store *dp, char *metric_name, time_t point_time, float value);
struct range_query_result get_range(struct data_store *dp, char *metric_name, time_t start, time_t end_time,  int *error);
void free_data_store(struct data_store *dp);
void free_range_query(struct range_query_result *query);

static float *load_from_db(struct data_store *dp, char *key, float *to, size_t len); 
static struct mill_file* init_file(char *f_name);
