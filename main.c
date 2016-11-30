#include <stdio.h>
#include "data_store.h"
#include "tcp_server.h"

int main() {
/*  struct data_store *st = create_data_store("/tmp/rocksdb_simple_example");
  int error;
  store_dp(st, "item_read_service", 1480387056, 1.3);
  float point = get_dp(st, "item_read_service", 1480387056, &error);
  printf("Data is %f.", point);
  struct range_query_result r_result = get_range(st, "item_read_service", 1480387056, 1480387058, &error);
  int start_date = r_result.start_date;
  for(int i = 0;  i < r_result.num_shards * r_result.shard_size; i++)
  {
    printf("[%d, %f], \n",  start_date++,  r_result.points[i]);
  }
  free_data_store(st);
  free_range_query(&r_result); */

  struct data_store *ds = create_data_store("/tmp/rocksdb_simple_example");
  struct tcp_server *server = tcp_server_create(5555);
  tcp_server_set_ds(server, ds);
  tcp_server_accept(server);
  tcp_server_cleanup(server);
  return 0;
}
