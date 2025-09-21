#include "hash_table.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void ht_entry_clear(ht_entry *e) {
  if (!e)
    return;

  free(e->k);
  free(e->v);
  e->k = NULL;
  e->v = NULL;
}

static ht_bucket *ht_get_bucket(ht_hash_table *ht, int bidx) {
  if (!ht)
    return NULL;

  if (bidx < 0 || bidx >= ht->len) {
    return NULL;
  }

  // might return NULL if bucket not initalized yet
  return ht->buckets[bidx];
}

static ht_bucket *ht_get_or_create_bucket(ht_hash_table *ht, int bidx) {
  if (!ht)
    return NULL;

  if (bidx < 0 || bidx >= ht->len) {
    return NULL;
  }

  ht_bucket *b = ht_get_bucket(ht, bidx);
  if (b)
    return b;

  // if bucket does not exist yet
  ht_bucket **ref = &ht->buckets[bidx];
  (*ref) = malloc(sizeof(ht_bucket));
  if (*ref == NULL) {
    return NULL;
  }

  (*ref)->size = 0;
  (*ref)->capacity = 4;
  (*ref)->data = calloc(((*ref)->capacity), sizeof(ht_entry));

  if (!(*ref)->data) {
    free(*ref);
    *ref = NULL;
    return NULL;
  }

  return *ref;
}

static ht_bucket *ht_resize_bucket(ht_hash_table *ht, int bidx) {
  ht_bucket *b = ht->buckets[bidx];

  int new_cap = b->capacity * 2;
  ht_entry *tmp = realloc(b->data, new_cap * sizeof(*tmp));
  if (!tmp) {
    return NULL;
  }

  b->data = tmp;
  b->capacity = new_cap;
  return b;
}

static int ht_find_in_bucket(ht_bucket *b, const char *k) {
  for (int i = 0; i < b->size; i++) {
    if (strcmp(b->data[i].k, k) == 0) {
      return i;
    }
  }
  return -1;
}

static void ht_erase_in_bucket(ht_bucket *b, int i) {
  if (!b || i < 0 || i >= b->size)
    return;

  free(b->data[i].k);
  free(b->data[i].v);

  if (i != b->size - 1) {
    b->data[i] = b->data[b->size - 1];
  }

  b->size--;
  b->data[b->size].k = NULL;
  b->data[b->size].v = NULL;
}

static void ht_bucket_insert(ht_bucket *b, const ht_entry *e) {
  b->data[b->size] = *e;
  b->size++;
}

static void ht_bucket_insert_override(ht_bucket *b, int i, const char *v) {
  char *nv = strdup_f(v);
  if (!nv)
    return;
  free(b->data[i].v);
  b->data[i].v = nv;
}

// https://stackoverflow.com/questions/7666509/hash-function-for-string
static unsigned long ht_hash(const unsigned char *s) {
  unsigned long hash = 5381;
  int c;

  while ((c = *s++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

static int ht_get_hash(ht_hash_table *ht, const unsigned char *s) {
  return ht_hash(s) % ht->len;
}

// hash table API

ht_hash_table *ht_new_hash_table() {
  ht_hash_table *ht = malloc(sizeof(ht_hash_table));
  if (!ht)
    return NULL;

  ht->len = 16;
  ht->count = 0;
  ht->buckets = calloc(ht->len, sizeof(ht_bucket *));

  if (!ht->buckets) {
    free(ht);
    return NULL;
  }

  return ht;
}

void ht_hash_table_clear(ht_hash_table *ht) {
  if (!ht)
    return;

  for (int i = 0; i < ht->len; i++) {
    ht_bucket *b = ht->buckets[i];

    if (b) {
      for (int j = 0; j < b->size; j++) {
        ht_entry_clear(&b->data[j]);
      }
      free(b->data);
      free(b);
      ht->buckets[i] = NULL;
    }
  }
  ht->count = 0;
}

void ht_hash_table_destroy(ht_hash_table *ht) {
  if (!ht)
    return;
  ht_hash_table_clear(ht);
  free(ht->buckets);
  free(ht);
}

void ht_insert(ht_hash_table *ht, const char *k, const char *v) {
  if (!ht || !k || !v)
    return;

  int bidx = ht_get_hash(ht, (const unsigned char *)k);

  // If key exists, override
  ht_bucket *b = ht_get_or_create_bucket(ht, bidx);
  if (!b)
    return;

  int idx = ht_find_in_bucket(b, k);
  if (idx == -1) {
    if (b->size == b->capacity && !ht_resize_bucket(ht, bidx))
      return;

    ht_entry e = {.k = strdup_f(k), .v = strdup_f(v)};

    if (!e.k || !e.v) {
      free(e.k);
      free(e.v);
      return;
    }
    ht_bucket_insert(b, &e);
    ht->count++;
  } else {
    ht_bucket_insert_override(b, idx, v);
  }
}

void ht_erase(ht_hash_table *ht, const char *k) {
  if (!ht || !k)
    return;

  int bidx = ht_get_hash(ht, (const unsigned char *)k);
  ht_bucket *b = ht_get_bucket(ht, bidx);
  if (!b)
    return;

  int i = ht_find_in_bucket(b, k);

  // Key does not exist
  if (i == -1)
    return;

  ht_erase_in_bucket(b, i);
  ht->count--;

  // Erase bucket to save from space
  if (b->size == 0) {
    free(b->data);
    free(ht->buckets[bidx]);
    ht->buckets[bidx] = NULL;
  }
}

// Returns internal pointer, may be invalid after hash table operations!
const char *ht_find(ht_hash_table *ht, const char *k) {
  if (!ht || !k)
    return NULL;

  int bidx = ht_get_hash(ht, (const unsigned char *)k);
  ht_bucket *b = ht_get_bucket(ht, bidx);
  if (!b)
    return NULL;

  int i = ht_find_in_bucket(b, k);
  if (i == -1)
    return NULL;

  return b->data[i].v;
}

void ht_print(ht_hash_table *ht) {
  printf("#---- Printing Hash Table Content -----\n");

  for (int i = 0; i < ht->len; i++) {
    if (ht->buckets[i] != NULL) {
      printf("| Bucket %d: ", i);
      for (int j = 0; j < ht->buckets[i]->size; j++) {
        ht_entry *e = &ht->buckets[i]->data[j];
        printf("[key: %s, value: %s] ", e->k, e->v);
      }
      printf("\n");
    }
  }
  printf("#---- Printed Hash Table Content -----\n\n");
}
