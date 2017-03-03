#include "data_store.h"
#include "circular_cache.h"

#define MMAP_CACHESIZE 100

static float const metric_list[SHARD_SIZE];

static void cleanup_cc(struct circular_cache* item) {
  free(item->key);
  munmap(item->value, SHARD_LEN(float)); 
  item->key = NULL;
}

static int metric_exists(char *base, char *name) {
  char *f_name;
  if (asprintf(&f_name,  "%s/%s", base, name)) {
    if( access( f_name, F_OK ) != -1 ) {
      return 1;
    } 
    free(f_name);
  }
  return 0;
}

struct data_store *create_data_store(char *db_path) {
	struct data_store *store = calloc(1, sizeof(struct data_store));
  int conn;
  store->circ_cache = calloc(MMAP_CACHESIZE, sizeof(struct circular_cache));
  store->msg_sock = nn_socket (AF_SP, NN_PUSH);
  if(store->msg_sock == -1) {
    printf("Can't create socket %d\n");
    return NULL;
  }
  store->m_folder = strdup(db_path);
  assert(store->m_folder);
  conn = nn_connect(store->msg_sock, "ipc:///tmp/test.ipc");
  assert(conn);
  if (conn <= 0) {
    printf("Can't connect to msg_sock\n");
    return NULL;
  }
  return store;
}

int store_dp(struct data_store *dp, char *metric_name, time_t point_time, float value) {
  assert(dp);
  assert(metric_name);
  assert(point_time > 0);
  char name[255];
  char *met_key;
  time_t week_s = get_week_start(point_time); 
  float *data_points;
  if (week_s ==  -1) {
    return -1;
  }
  sprintf(name, METRIC_FORMAT, metric_name, week_s); 
  data_points = load_from_db(dp, name, SHARD_LEN(float));
  if (data_points) {
    data_points[point_time  - week_s] = value;
  } else {
    return -1;
  }
  if (asprintf(&met_key, "met-%s", metric_name) != -1) {
    send_msg_to_master(dp, met_key);
    free(met_key);
  }
  return 0;
}
static struct mill_file* init_file(char *f_name) {
  assert(f_name);
  struct mill_file *fd = mfopen(f_name, O_RDWR | O_CREAT | O_NOFOLLOW | O_EXCL, S_IRUSR | S_IWUSR);
  int i = 0;
  float f = 0.0;
  if(fd)
  {
    for(i = 0; i < SHARD_SIZE; i++) {
      mfwrite(fd, &f, sizeof(float), -1);
    }
  } else {
    fd = mfopen(f_name, O_RDWR | O_CREAT | O_NOFOLLOW, S_IRUSR | S_IWUSR);
  }
  return fd;
}
static int send_msg_to_master(struct data_store *dp, char *msg) {
  assert(dp);
  assert(msg);

  int sz_msg = strlen (msg) + 1;
  int bytes = nn_send (dp->msg_sock, msg, sz_msg, 0);
  assert (bytes == sz_msg);
  return bytes == sz_msg ? 1 : -1;
}
static float *load_from_db(struct data_store *dp, char *key, size_t len) {
  assert(dp);
  assert(key);
  assert(len);
  char *f_name = NULL;
  struct mill_file *fd;
  float *data = NULL;
  int error_no;
  struct circular_cache *cc;
  if ((cc = circular_cache_find(dp->circ_cache, key, MMAP_CACHESIZE))) {
    return (float *) cc->value;
  }
  asprintf(&f_name, "%s/%s", dp->m_folder,  key);
  fd = init_file(f_name);
  if(!fd)
  {
    printf("Can't create file %s", f_name);
    goto cleanup;
  }
  data = mmap(0, len, PROT_WRITE | PROT_READ, MAP_SHARED, fd->fd, 0);
  if(data == MAP_FAILED) {
    perror("Can't map file (load from DB).");
    goto cleanup;
  } else {
    if(circular_cache_add(dp->circ_cache, key, data, MMAP_CACHESIZE) == -1) {
      circular_cache_cleanup(dp->circ_cache, MMAP_CACHESIZE, cleanup_cc);
    }
  }
  error_no = 0;
cleanup:
  if (fd) {
    mfflush(fd, -1);
    mfclose(fd);
  }
  if (f_name) {
    free(f_name);
  }
  return data == MAP_FAILED || error_no > 0 ? NULL : data;
}


time_t init_ds_iter(time_t p_time) {
  return get_week_start(p_time);
}
time_t incr_ds_iter(time_t s, time_t length) {
  return s + length;
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
  if (metric_exists(dp->m_folder, name)) {
    r_result.points     = load_from_db(dp, name, SHARD_LEN(float));
  } else {
    r_result.points     = (float *) metric_list;
  }
  r_result.s_type       = DS_MMAP;
  *error = 0;
  free(name);
  return r_result;
}

void free_data_store(struct data_store *dp) {
  if (dp->msg_sock >= 0) 
     nn_shutdown (dp->msg_sock, 0);
	if (dp->db)
		 free(dp->db);
	if (dp->options)
		 free(dp->options);
	if (dp->writeoptions)
		 free(dp->writeoptions);
	if (dp->readoptions)
		 free(dp->readoptions);
  if (dp->circ_cache) 
    free(dp->circ_cache);
  if (dp->m_folder) 
     free(dp->m_folder);
	if (dp) 
		free(dp);
}

void free_range_query(struct range_query_result *query) {
  if(query->s_type == DS_MALLOC) {
    free(query->points);
  } 
}


