#pragma once 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <libmill.h>
#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>
#include <rocksdb/c.h>
#include <unistd.h>

struct master_node {
  int sock;
	rocksdb_t *db;
	rocksdb_writeoptions_t *writeoptions;
	rocksdb_readoptions_t *readoptions;

};

struct master_node *create_master_node(char *url, char *db_path);
int master_node_accept_connect(struct master_node *node);
void free_master_node(struct master_node *node);
