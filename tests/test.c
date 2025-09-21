#include "test.h"
#include "hash_table.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* helper functions */
static void expect_str_eq(const char *got, const char *want, const char *msg) {
  if (got == NULL) {
    fprintf(stderr, "fail: %s (got=NULL, want=\"%s\")\n", msg, want);
    assert(0);
  }
  if (strcmp(got, want) != 0) {
    fprintf(stderr, "fail: %s (got=\"%s\", want=\"%s\")\n", msg, got, want);
    assert(0);
  }
}

static void expect_null(const char *got, const char *msg) {
  if (got != NULL) {
    fprintf(stderr, "fail: %s (expected NULL, got \"%s\")\n", msg, got);
    assert(0);
  }
}

/* entrypoint */
void ht_run_all_tests() {
  printf("=== basic insert find erase ===\n");
  ht_hash_table *ht = ht_new_hash_table(0);
  assert(ht != NULL);

  ht_insert(ht, "hello", "world");
  ht_insert(ht, "name", "hasan");
  ht_insert(ht, "age", "46");

  // ht_print(ht);

  expect_str_eq(ht_find(ht, "hello"), "world", "find hello");
  expect_str_eq(ht_find(ht, "name"), "hasan", "find name");
  expect_str_eq(ht_find(ht, "age"), "46", "find age");
  expect_null(ht_find(ht, "missing"), "find missing key returns null");

  ht_insert(ht, "age", "47");
  // ht_print(ht);
  expect_str_eq(ht_find(ht, "age"), "47", "overwrite key updates value");

  ht_erase(ht, "name");
  // ht_print(ht);
  expect_null(ht_find(ht, "name"), "after erase name not found");

  ht_erase(ht, "not-there");
  expect_str_eq(ht_find(ht, "hello"), "world", "erase missing leaves intact");

  printf("=== clear reuse ===\n");
  ht_hash_table_clear(ht);
  // ht_print(ht);
  expect_null(ht_find(ht, "hello"), "after clear hello not found");
  expect_null(ht_find(ht, "age"), "after clear age not found");

  ht_insert(ht, "a", "1");
  ht_insert(ht, "b", "2");
  // ht_print(ht);
  expect_str_eq(ht_find(ht, "a"), "1", "reuse after clear a");
  expect_str_eq(ht_find(ht, "b"), "2", "reuse after clear b");

  ht_erase(ht, "a");
  ht_erase(ht, "b");
  // ht_print(ht);
  expect_null(ht_find(ht, "a"), "after erase a not found");
  expect_null(ht_find(ht, "b"), "after erase b not found");

  printf("=== bulk insert find erase ===\n");
  ht_hash_table *small = ht_new_hash_table(2);
  assert(small != NULL);

  char key[64], val[64];
  for (int i = 0; i < 100; i++) {
    snprintf(key, sizeof key, "k%03d", i);
    snprintf(val, sizeof val, "v%03d", i);
    ht_insert(small, key, val);
  }

  // ht_print(small);

  expect_str_eq(ht_find(small, "k000"), "v000", "bulk find k000");
  expect_str_eq(ht_find(small, "k042"), "v042", "bulk find k042");
  expect_str_eq(ht_find(small, "k099"), "v099", "bulk find k099");

  ht_insert(small, "k042", "answer");
  // ht_print(small);
  expect_str_eq(ht_find(small, "k042"), "answer", "bulk overwrite k042");

  for (int i = 0; i < 100; i += 2) {
    snprintf(key, sizeof key, "k%03d", i);
    ht_erase(small, key);
  }
  // ht_print(small);

  for (int i = 0; i < 100; i++) {
    snprintf(key, sizeof key, "k%03d", i);
    const char *found = ht_find(small, key);
    if (i % 2 == 0) {
      if (found != NULL) {
        fprintf(stderr, "fail: expected k%03d erased, got \"%s\"\n", i, found);
        assert(0);
      }
    } else {
      if (i == 42) {
        expect_str_eq(found, "answer", "bulk post erase overwrite k042");
      } else {
        snprintf(val, sizeof val, "v%03d", i);
        expect_str_eq(found, val, "bulk post erase odd key");
      }
    }
  }

  printf("=== edge cases ===\n");
  ht_hash_table *edge = ht_new_hash_table(0);
  assert(edge != NULL);

  expect_null(ht_find(edge, "nope"), "find in empty returns null");
  ht_erase(edge, "nope");

  char long_key[1024];
  memset(long_key, 'x', sizeof(long_key) - 1);
  long_key[sizeof(long_key) - 1] = '\0';
  ht_insert(edge, long_key, "longval");
  expect_str_eq(ht_find(edge, long_key), "longval", "find long key");

  ht_insert(edge, "spaced key", "val1");
  ht_insert(edge, "key-with-!@#", "val2");
  // ht_print(edge);
  expect_str_eq(ht_find(edge, "spaced key"), "val1", "find spaced key");
  expect_str_eq(ht_find(edge, "key-with-!@#"), "val2", "find special key");

  ht_hash_table_clear(edge);
  ht_hash_table_clear(edge);
  // ht_print(edge);
  expect_null(ht_find(edge, "spaced key"), "after clear no key");
  ht_insert(edge, "new", "val");
  // ht_print(edge);
  expect_str_eq(ht_find(edge, "new"), "val", "insert after repeated clear");

  ht_hash_table_destroy(edge);
  // this is user's responsibility btw!!
  edge = NULL;
  ht_hash_table_destroy(edge);

  ht_hash_table_clear(small);
  ht_hash_table_destroy(small);
  ht_hash_table_destroy(ht);

  printf("all tests passed\n");
}
