#include "hash_table.h"
#include "string.h"
#include <assert.h>

int main() {
  ht_hash_table *ht = ht_new_hash_table();
  char *k = "Hello";
  char *v = "World";

  ht_insert(ht, k, v);
  ht_insert(ht, "name", "hasan");
  ht_insert(ht, "age", "46");

  ht_print(ht);

  ht_erase(ht, "name");
  ht_erase(ht, "age");

  ht_print(ht);

  const char *val = ht_find(ht, k);

  assert(strcmp(v, val) == 0);

  ht_erase(ht, "Hello");

  ht_print(ht);

  ht_hash_table_destroy(ht);

  return 0;
}
