#include "data_store.h"

struct data_store *create_data_store(char *db_path) {
  char *err = NULL;
	struct data_store *store = calloc(1, sizeof(struct data_store));
  store->options    = rocksdb_options_create();

  long cpus = sysconf(_SC_NPROCESSORS_ONLN);  
  rocksdb_options_increase_parallelism(store->options, (int)(cpus));
  rocksdb_options_optimize_level_style_compaction(store->options, 0);
  rocksdb_options_set_create_if_missing(store->options, 1);

  store->db = rocksdb_open(store->options, db_path, &err);
	if (err) 
  {
    printf("Error opening db %s", err);
		return NULL;
  }

  store->writeoptions = rocksdb_writeoptions_create();
  store->readoptions = rocksdb_readoptions_create();

  return store;
}

int store_dp(struct data_store *dp, char *metric_name, time_t point_time, float value) {
  printf("Start storing data\n");
  char name[255];
  time_t week_s = get_week_start(point_time); 
  printf("Week start %ld.\n", week_s);
  float *data_points;
  if (week_s ==  -1) {
    return -1;
  }
  sprintf(name, METRIC_FORMAT, metric_name, week_s); 
  printf("Metric Name /%s/\n", name);
  data_points = load_from_db(dp, name);
  if ( data_points == NULL )
  {
    printf("Data points is null\n");
    data_points = calloc(SHARD_SIZE, sizeof(float));
  }
  else 
  {
    printf("Loading data from db.\n");
  }
  data_points[point_time  - week_s] = value;
  save_to_db(dp, name, data_points);
  return 0;
}

static char *save_to_db(struct data_store *dp, char *key, float *points) {
 char *err = NULL;
 rocksdb_put(dp->db, dp->writeoptions, key, strlen(key), (const char *) points, SHARD_SIZE * sizeof(float), &err);
 if (err) {
  printf("Error putting data %s.\n", err);
	return err;
 }
 return NULL;
}

static float *load_from_db(struct data_store *dp, char *key) {
	size_t len;
	char *err = NULL;
  float *returned_value = (float *) rocksdb_get(dp->db, dp->readoptions, key, strlen(key), &len, &err);
	if (err) 
  {
    printf("Error loading the data %s", err);
		return NULL;
  }
  printf("length will be %zu.", len);
	return returned_value;
}

struct range_query_result get_range(struct data_store *dp, char *metric_name, time_t start, time_t end,  int *error) {
  time_t week_s = get_week_start(start); 
  time_t week_e = get_week_start(end); 
  double sec_diff = difftime(week_e, week_s); 
  struct range_query_result r_result;
  float *points = NULL;
  int num_shards = 1;
  if (week_s ==  -1 || week_e == -1) {
    *error = -1;
    return r_result;
  }
  if (sec_diff != 0 && sec_diff > SHARD_SIZE)
  { 
     num_shards = (sec_diff/ SHARD_SIZE);
  }
  r_result.num_shards = num_shards;
  r_result.shard_size = SHARD_SIZE;
  r_result.start_date = week_s;
  r_result.points = calloc(SHARD_SIZE * num_shards, sizeof(float));
  if (!r_result.points) {
    *error = -2;
    return r_result;
  }
  int num_week = 0;
  for(time_t start = week_s; start <= week_e; start = incr_week(start), num_week++) {
    char name[255];
    sprintf(name, METRIC_FORMAT, metric_name, start); 
    printf("\nDATA STORE metric name %s\n", name);
    float *data_points = load_from_db(dp, name);
    if (data_points)
    {
      memcpy(r_result.points + num_week, data_points, SHARD_SIZE * sizeof(float));
    }
  }
  *error = 0;
  return r_result;
}
float get_dp(struct data_store *dp, char *metric_name, time_t point_time, int *error) {
  time_t week_s = get_week_start(point_time); 
  char name[255];
  if (week_s ==  -1) {
    *error = -1;
    return 0;
  }
  sprintf(name, METRIC_FORMAT, metric_name, week_s); 
  float *data_points = load_from_db(dp, name);
  if ( data_points == NULL )
  {
    printf("Creating data point\n");
    data_points = calloc(SHARD_SIZE, sizeof(float));
  }
  else 
  {
    printf("Getting one point from db\n");
  }
  *error = 0;
  return data_points[point_time  - week_s];
}

void free_data_store(struct data_store *dp) {
	if(dp->db)
		free(dp->db);

	if(dp->options)
		free(dp->options);

	if(dp->writeoptions)
		free(dp->writeoptions);

	if(dp->readoptions)
		free(dp->readoptions);

	if (dp) 
		free(dp);
}

void free_range_query(struct range_query_result *query) {
  free(query->points);
}


