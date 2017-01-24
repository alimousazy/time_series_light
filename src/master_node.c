#include "master_node.h"

static int getfd (int s) {
  int rc, fd;
  size_t fdsz = sizeof fd;
  if ( nn_getsockopt (s, NN_SOL_SOCKET, NN_RCVFD, &fd, &fdsz) != 0 )
    return -1;
  return fd;
}

static coroutine void meta_data_handler (struct master_node *node) {
  while (1)
  {
    char *value = "1";
    char *key =  NULL;
    char *buf = NULL;
    char *err = NULL;
    int raw_sock_id;
    size_t sock_size = sizeof(raw_sock_id);
    int events = fdwait(getfd(node->sock), FDW_IN, -1);
    int bytes = nn_recv (node->sock, &buf, NN_MSG, 0);
    //temp
    key = buf;
    rocksdb_put(node->db, node->writeoptions, key, strlen(key), value, strlen(value) + 1, &err);
    //temp
    key = NULL;
    assert(!err);
    assert (bytes >= 0);
cleanup: 
      if (buf) 
        nn_freemsg (buf);
      if ( key ) 
        free(key);
  }
}
struct master_node *create_master_node(char *url, char *db_path) {
  struct master_node *node = calloc(1, sizeof(struct master_node));
  long cpus = sysconf(_SC_NPROCESSORS_ONLN);  
  char *err = NULL;
	rocksdb_options_t *options = rocksdb_options_create();

  node->sock = nn_socket (AF_SP, NN_PULL);
  if(nn_bind (node->sock, url) < 0) {
    return NULL;
  }
  node->meta_server =  http_server_create(8080);
  rocksdb_options_increase_parallelism(options, (int)(cpus));
  rocksdb_options_optimize_level_style_compaction(options, 0);
  rocksdb_options_set_create_if_missing(options, 1);
  rocksdb_options_set_prefix_extractor(options, rocksdb_slicetransform_create_fixed_prefix(7));
  node->db = rocksdb_open(options, db_path, &err);
	if (err) 
  {
		return NULL;
  }
  node->writeoptions = rocksdb_writeoptions_create();
  node->readoptions = rocksdb_readoptions_create();

  node->meta_server->db = node->db;
  node->meta_server->readoptions = node->readoptions;
  return  node;
}

int master_node_accept_connect(struct master_node *node) {
  go(http_server_accept(node->meta_server));
  go(meta_data_handler(node));
  return 0;
}

void free_master_node(struct master_node *node) {
  free(node);
  http_server_cleanup(node->meta_server);
}
