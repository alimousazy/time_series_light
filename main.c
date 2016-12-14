#include <stdio.h>
#include <assert.h>
#include "data_store.h"
#include "tcp_server.h"
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include "master_node.h"

int main() {
  struct tcp_server *server = tcp_server_create(5555);
  long   cpus = sysconf(_SC_NPROCESSORS_ONLN);  
  pid_t pid;
  int i = 0;
  for (i = 0; i < cpus - 1; ++i) {
		pid = mfork();
		if(pid < 0) {
			 perror("Can't create new process");
			 return 1;
		}
		if(pid > 0) {
				break;
		}
  }

  if (pid > 0) {
    sleep(10);
    printf("I'm slave \n");
    struct data_store *ds = create_data_store("/tmp/rocksdb_simple_example");
    tcp_server_set_ds(server, ds);
    tcp_server_accept(server);
    tcp_server_cleanup(server);
  } else if (pid == 0) {
    printf("I'm the master \n");
    struct master_node *node = create_master_node("ipc:///tmp/test.ipc", "/tmp/rocksdb_simple_example");
    if (!node) {
      printf("Error can't create master node");
      return -1;
    }

    printf("accepting \n");
    master_node_accept_connect(node);
    free_master_node(node);
  }
  return 0;
}
