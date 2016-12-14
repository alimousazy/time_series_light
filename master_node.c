#include "master_node.h"

static int nn_mill_getfd (int s) {
  int rc, fd;
  size_t fdsz = sizeof fd;

  if ( nn_getsockopt (s, NN_SOL_SOCKET, NN_RCVFD, &fd, &fdsz) != 0 )
    return -1;

  /* TODO: we might as well return both FDs, NN_SNDFD too */
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
    printf("Waiting for meta \n");
    int events = fdwait(nn_mill_getfd(node->sock), FDW_IN, -1);
    printf("done Waiting for meta \n");
    int bytes = nn_recv (node->sock, &buf, NN_MSG, 0);
    printf(" fdsafdasf \n");
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
  return  node;
}

int master_node_accept_connect(struct master_node *node) {
  go(meta_data_handler(node));
  return 0;
}

void free_master_node(struct master_node *node) {
  free(node);
}
