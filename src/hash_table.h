#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct {
  char *k;
  char *v;
} ht_entry;

typedef struct {
  ht_entry *data; // Use array here because caching >>> on arrays compared to ll
  int size;
  int capacity;
} ht_bucket;

typedef struct {
  ht_bucket **buckets;
  int len;
  int count;
} ht_hash_table;

ht_hash_table *ht_new_hash_table(int n);
void ht_hash_table_clear(ht_hash_table *ht);
void ht_hash_table_destroy(ht_hash_table *ht);

void ht_insert(ht_hash_table *ht, const char *k, const char *v);
void ht_erase(ht_hash_table *ht, const char *k);
const char *ht_find(ht_hash_table *ht, const char *k);

void ht_print(ht_hash_table *ht);

#endif