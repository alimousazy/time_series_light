#pragma once

#include "time_convert.h"
#include <rocksdb/c.h>
#include <unistd.h> 
#include <string.h>
#include <time.h>

#define METRIC_FORMAT  "%s-%ld"
#define DB_NAME "time_series"
#define SHARD_SIZE 604800

struct data_store {
	rocksdb_t *db;
	rocksdb_options_t *options;
	rocksdb_writeoptions_t *writeoptions;
	rocksdb_readoptions_t *readoptions;
};

struct range_query_result {
  float *points;
  int num_shards;
  time_t shard_size;
  time_t start_date;
};

//public method
struct data_store *create_data_store();
int store_dp(struct data_store *dp, char *metric_name, time_t point_time, float value);
float get_dp(struct data_store *dp, char *metric_name, time_t point_time, int *error);
struct range_query_result get_range(struct data_store *dp, char *metric_name, time_t start, time_t end,  int *error);
void free_data_store(struct data_store *dp);
void free_range_query(struct range_query_result *query);


static char *save_to_db(struct data_store *dp, char *key, float *points);
static float *load_from_db(struct data_store *dp, char *key); 
