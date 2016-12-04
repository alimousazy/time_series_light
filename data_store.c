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
//printf("Error opening db %s", err);
		return NULL;
  }

  store->writeoptions = rocksdb_writeoptions_create();
  store->readoptions = rocksdb_readoptions_create();

  return store;
}

int store_dp(struct data_store *dp, char *metric_name, time_t point_time, float value) {
  char name[255];
  time_t week_s = get_week_start(point_time); 
  float *data_points;
  if (week_s ==  -1) {
    return -1;
  }
  sprintf(name, METRIC_FORMAT, metric_name, week_s); 
  data_points = load_from_db(dp, name, NULL, SHARD_LEN(float));
  if (data_points) {
    data_points[point_time  - week_s] = value;
  }
  return 0;
}
static struct mill_file* init_file(char *f_name) {
  struct mill_file *fd = mfopen(f_name, O_RDWR | O_CREAT | O_NOFOLLOW | O_EXCL, S_IRUSR | S_IWUSR);
  int i = 0;
  float f = 0.0;
  if(fd)
  {
//printf("\nWriting zeorrrrrs to the file \n");
    for(i = 0; i < SHARD_SIZE; i++) {
      mfwrite(fd, &f, sizeof(float), -1);
    }
  } else {
    fd = mfopen(f_name, O_RDWR | O_CREAT | O_NOFOLLOW, S_IRUSR | S_IWUSR);
  }
  return fd;
}
static float *load_from_db(struct data_store *dp, char *key, float *to, size_t len) {
  char *f_name = NULL;
  struct mill_file *fd;
  float *data = NULL;
  int error_no;
  asprintf(&f_name, "./cache/%s", key);
  fd = init_file(f_name);
//printf("opening file\n");

  if(!fd)
  {
//printf("can't open file %s\n", key);
    goto cleanup;
  }
//printf("mappping %s\n", key);
  data = mmap(0, len, PROT_WRITE | PROT_READ, MAP_SHARED, fd->fd, 0);
  if(data == MAP_FAILED) {
    perror("Can't map file (load from DB).");
    goto cleanup;
  }
//printf("map done success read\n");
  error_no = 0;
cleanup:
  if (fd) {
    mfclose(fd);
  }
  if (f_name) {
    free(f_name);
  }
  if (error_no == 0 && to) {
    memcpy(to, data, len);
  }
  return data == MAP_FAILED ? NULL : data;
}


struct range_query_result ds_current(struct data_store *dp, char *metric_name, time_t t_time,  int *error) {
  time_t week_s = get_week_start(t_time); 
  struct range_query_result r_result;
  char *name;
  if (week_s ==  -1) {
    *error = -1;
    return r_result;
  }
  r_result.num_shards = 1;
  r_result.shard_size = SHARD_SIZE;
  r_result.start_date = week_s;
  asprintf(&name, METRIC_FORMAT, metric_name, week_s); 
  r_result.points     = load_from_db(dp, name, NULL, SHARD_LEN(float));
  r_result.s_type       = DS_MMAP;
  *error = 0;
  printf("returing result \n");
  free(name);
  return r_result;
}

struct range_query_result get_range(struct data_store *dp, char *metric_name, time_t start, time_t end_time,  int *error) {
  time_t week_s = get_week_start(start); 
  time_t week_e = get_week_start(end_time); 
  struct range_query_result r_result;
  float *points = NULL;
  int num_shards = 1;
  time_t tmp = week_s; 
  if (week_s ==  -1 || week_e == -1) {
    *error = -1;
    return r_result;
  }
  while(tmp != week_e) {
    tmp =  get_week_start(tmp + SHARD_SIZE + 10);
    num_shards++;
  }
  r_result.num_shards = num_shards;
  r_result.shard_size = SHARD_SIZE;
  r_result.start_date = week_s;
  r_result.s_type       = DS_MALLOC;
  r_result.points = calloc(SHARD_SIZE * num_shards, sizeof(float));
  if (!r_result.points) {
    *error = -2;
    return r_result;
  }
  int num_week = 0;
  for(time_t start = week_s; start <= week_e; start = incr_week(start, SHARD_SIZE), num_week++) {
    char name[255];
    float *data;
    sprintf(name, METRIC_FORMAT, metric_name, start); 
    data = load_from_db(dp, name, r_result.points + (num_week * SHARD_SIZE), SHARD_LEN(float));
    if ( data ) {
      munmap(data, SHARD_LEN(float)); 
    }
  }
  *error = 0;
  printf("returing result \n");
  return r_result;
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
  if(query->s_type == DS_MALLOC) {
    free(query->points);
  } else if(query->s_type == DS_MMAP) {
    munmap(query->points, query->shard_size * sizeof(float));
  }
}


