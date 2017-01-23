#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <time.h>

struct range_query_result {
  float *points;
  int num_shards;
  time_t shard_size;
  time_t start_date;
  int s_type; 
};

struct M_ITEM {
  int t_time;
  char *name;
  float value;
};

struct M_ITEM metric_list[] = {
  {1483063128, "metric_1", 12229.1},
  {1482112728, "metric_2", 20.1},
  {1481162328, "metric_3", -3.1},
  {1464832728, "metric_4", 120000}
};


struct data_store;
extern struct range_query_result ds_current(struct data_store *dp, char *metric_name, time_t t_time,  int *error);
extern int store_dp(struct data_store *dp, char *metric_name, time_t point_time, float value);
extern struct data_store *create_data_store(char *);
extern time_t init_ds_iter(time_t p_time);

//mock nn_connect
int nn_connect(int x, char *path) {
  assert_true(x > 0);
  assert_non_null(path);
  assert_true(strlen(path) > 0);
  return (int) mock();
}

int nn_socket(int x, int l) {
  return (int) mock();
}

int nn_send(int x, char *l, int msg_size, int flags) {
  if ( x == -1 )
    return -1;
  return msg_size;
}




//Add new element
void test_creating_data_store(void **state) {
  will_return(nn_connect, 1);
  will_return(nn_socket, 1);
  struct data_store *dt =  create_data_store("/tmp/");
  assert_non_null(dt);
}

void test_creating_data_store_cant_connect(void **state) {
  will_return(nn_connect, -1);
  will_return(nn_socket, 1);
  struct data_store *dt =  create_data_store("/tmp/cache");
  assert_null(dt);
}

void test_creating_data_store_cant_create_socket(void **state) {
  will_return(nn_socket, -1);
  struct data_store *dt =  create_data_store("/tmp/cache");
  assert_null(dt);
}

void test_storing_data_point(void **state) {
  will_return(nn_connect, 1);
  will_return(nn_socket, 1);
  struct data_store *dt =  create_data_store("/tmp/cache");
  int n = 0;
  int test_num = 0;
  for (n = 0; n < sizeof(metric_list) / sizeof(metric_list[0]); n++) {
    int result = store_dp(dt, metric_list[n].name, metric_list[n].t_time, metric_list[n].value);
    assert_true(result > -1);
  }
}

void test_retrieve_data_point(void **state) {
  will_return(nn_connect, 1);
  will_return(nn_socket, 1);
  struct range_query_result r;
  int n = 0;
  int error;
  struct data_store *dt =  create_data_store("/tmp/cache");
  for (n = 0; n < sizeof(metric_list) / sizeof(metric_list[0]); n++) {
    time_t p_time = metric_list[n].t_time;
    time_t s_start = init_ds_iter(p_time);
    r = ds_current(dt, metric_list[n].name, s_start, &error);
    assert_true(r.points[p_time - r.start_date]  ==  metric_list[n].value);
  }
}

int main(int argc, char *argv[]) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_creating_data_store),
    cmocka_unit_test(test_creating_data_store_cant_create_socket),
    cmocka_unit_test(test_creating_data_store_cant_connect),
    cmocka_unit_test(test_storing_data_point),
    cmocka_unit_test(test_retrieve_data_point),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
