#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "../src/circular_cache.h"

struct circular_cache circular_cache_list[2];

extern struct circular_cache *circular_cache_find(struct circular_cache *list, char *key, int length);
extern int circular_cache_add(struct circular_cache *list, char *key, void *value, int length);
extern void circular_cache_cleanup(struct circular_cache *list, int length, circular_cache_cleanup_cb func);

int rand(void) {
  return (int) mock();
}


static void free_call_back(struct circular_cache *item) {
  assert(item);
}

//Add new element
void test_circular_cache_add_one(void **state) {
  int added = circular_cache_add(circular_cache_list, "one", "one_value", sizeof(circular_cache_list)/ sizeof(circular_cache_list[0]));
  assert_int_equal(added, 0);
}

void test_circular_cache_add_two(void **state) {
  int added = circular_cache_add(circular_cache_list, "two", "two_value", sizeof(circular_cache_list)/ sizeof(circular_cache_list[0]));
  assert_int_equal(added, 0);
}

//Add new element will fail since buffer list is full
void test_circular_cache_add_full(void **state) {
  int added = circular_cache_add(circular_cache_list, "one_fail", "one_value", sizeof(circular_cache_list)/ sizeof(circular_cache_list[0]));
  assert_int_equal(added, -1);
}

void test_circular_cache_find_two_not_exists(void **state) {
  struct circular_cache *result = circular_cache_find(circular_cache_list, "two", sizeof(circular_cache_list)/ sizeof(circular_cache_list[0]));
  assert_null(result);
}

/* This test case will fail but the assert is caught by run_tests() and the
 * next test is executed. */
void test_circular_cache_find_one(void **state) {
  struct circular_cache *result = circular_cache_find(circular_cache_list, "one", sizeof(circular_cache_list)/ sizeof(circular_cache_list[0]));
  assert_non_null(result);
}

void test_circular_cache_find_two(void **state) {
  struct circular_cache *result = circular_cache_find(circular_cache_list, "two", sizeof(circular_cache_list)/ sizeof(circular_cache_list[0]));
  assert_non_null(result);
}

void test_circular_cache_find_three(void **state) {
  struct circular_cache *result = circular_cache_find(circular_cache_list, "three", sizeof(circular_cache_list)/ sizeof(circular_cache_list[0]));
  assert_non_null(result);
}

// successfull clean up 
void test_circular_cache_cleanup_two(void **state) {
  will_return(rand, 1);
  circular_cache_cleanup(circular_cache_list, 2, free_call_back);
  assert_null(circular_cache_list[1].key);
}

// add after success cleanup
void test_circular_cache_add_three(void **state) {
  int added = circular_cache_add(circular_cache_list, "three", "three_value", sizeof(circular_cache_list)/ sizeof(circular_cache_list[0]));
  assert_int_equal(added, 0);
}



int main(int argc, char *argv[]) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_circular_cache_add_one),
    cmocka_unit_test(test_circular_cache_add_two),
    cmocka_unit_test(test_circular_cache_add_full),
    cmocka_unit_test(test_circular_cache_find_one),
    cmocka_unit_test(test_circular_cache_find_two),
    cmocka_unit_test(test_circular_cache_cleanup_two),
    cmocka_unit_test(test_circular_cache_find_two_not_exists),
    cmocka_unit_test(test_circular_cache_add_three),
    cmocka_unit_test(test_circular_cache_find_three),
    cmocka_unit_test(test_circular_cache_find_one),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
