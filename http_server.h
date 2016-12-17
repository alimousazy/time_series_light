#pragma once

#include "util.h"

#include <errno.h>
#include <rocksdb/c.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libmill.h>
#include <assert.h>


#define HTTP_SERVER_YIELD_EVERY 1000

struct http_server {
	int port;
	tcpsock sk;
  struct data_store *ds;
	rocksdb_t *db;
	rocksdb_readoptions_t *readoptions;
};

static const int http_server_timeout = 10000;

struct http_server *http_server_create(int port);	
void http_server_accept(struct http_server* server);
static void http_process_data(struct http_server* server, tcpsock sk); 
void http_server_cleanup(struct http_server *server);
