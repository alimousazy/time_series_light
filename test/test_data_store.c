#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <time.h>

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
extern int store_dp(struct data_store *dp, char *metric_name, time_t point_time, float value);
extern struct data_store *create_data_store(char *);

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
  int test_num = 0;
  for (int n = 0; n < sizeof(metric_list[0]) / sizeof(metric_list[0]); n++) {
    int result = store_dp(dt, metric_list[n].name, metric_list[n].t_time, metric_list[n].value);
    assert_true(result > -1);
  }
}

int main(int argc, char *argv[]) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_creating_data_store),
    cmocka_unit_test(test_creating_data_store_cant_create_socket),
    cmocka_unit_test(test_creating_data_store_cant_connect),
    cmocka_unit_test(test_storing_data_point),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
