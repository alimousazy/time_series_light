#include "http_server.h"

struct http_server *http_server_create(int port) {	
	struct http_server *server; 
	if( port < 0 ) {
		return NULL;
	}
	server = malloc(sizeof(struct http_server));
	if (!server)
		return NULL;
	ipaddr addr = iplocal(NULL, port, 0);
  server->sk   = tcplisten(addr, 10);
	if (!server->sk) {
		free(server);
		return NULL;
	}
	return server;
}

coroutine void http_server_accept(struct http_server* server) {
	while (1) {
		tcpsock as = tcpaccept(server->sk, -1);
    if(!as)
       return;
		go(http_process_data(server, as));
	}
}

static void process_line(struct http_server *server, char *inbuf, tcpsock sk) {
  int const part_num = 2;
  char *parts[part_num];
  int64_t deadline;
  rocksdb_iterator_t *it = rocksdb_create_iterator(server->db, server->readoptions);
  int num = util_process_line(inbuf, parts, ":", part_num);
  if (num != 2) {
    tcpsend(sk, "ERROR", 5, -1);
  } else {
    size_t key_len = 0;
    long r_count = 0;
    rocksdb_iter_seek(it, parts[1], strlen(parts[1]));
    while (rocksdb_iter_valid(it)) {
      deadline = now() + http_server_timeout;
      const char *key = rocksdb_iter_key(it, &key_len);
      tcpsend(sk, key, key_len, deadline);
      tcpsend(sk, ",", 1, deadline);
      rocksdb_iter_next(it);
      r_count++;
    }
    if(r_count == 0) {
      const char *no_result = "no-result";
      tcpsend(sk, no_result, strlen(no_result), deadline);
    }
  }
  deadline = now() + http_server_timeout;
  rocksdb_iter_destroy(it);
  tcpflush(sk, -1);
}

static coroutine void http_process_data(struct http_server* server, tcpsock sk) {
  int64_t deadline = now() + http_server_timeout;
	char inbuf[250];
  int buf_size = sizeof(inbuf) - 1;
	while (buf_size > 0) {
    char *parts[3];   
    int num_process = 0;
		size_t sz = tcprecvuntil(sk, inbuf, buf_size, "\n", 1, deadline);
    inbuf[sz - 1] = '\0';
		if(errno != 0)
 	 	  goto cleanup;
    process_line(server, inbuf, sk);
	}
	cleanup:
		tcpclose(sk);
}

void http_server_cleanup(struct http_server *server)
{
	tcpclose(server->sk);
	free(server);
}
