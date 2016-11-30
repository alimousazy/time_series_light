#pragma once

#include "data_store.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libmill.h>



struct tcp_server {
	int port;
	tcpsock sk;
  struct data_store *ds;
};

static const int tcp_server_timeout = 10000;

struct tcp_server *tcp_server_create(int port);	
void tcp_server_set_ds(struct tcp_server *server, struct data_store *ds); 
int tcp_server_accept(struct tcp_server* server);
void tcp_process_data(struct tcp_server* server, tcpsock sk); 
void tcp_server_cleanup(struct tcp_server *server);
