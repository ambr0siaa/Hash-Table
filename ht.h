#ifndef HT_H_
#define HT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HT_MAGIC    0x428a2f98    // cubic root of 2
#define HT_CAPACITY 69

typedef long long int i64;
typedef i64 (*hash_func_t) (const char *s);

// sqrt of [2, 3, 5, 7, 11, 13, 17, 19]
static const i64 k[8] = { 
    0x6a09e667, 0xbb67ae85,0x510e527f, 0x9b05688c,
    0x1f83d9ab, 0x5be0cd19, 0x3c6ef372, 0xa54ff53a
};

struct ht_item {
    const char *key;
    i64 value;
    i64 hash;
};

struct bucket {
    struct ht_item *item;
    struct bucket *next;
};

typedef struct Hash_Table Hash_Table;

struct Hash_Table {
    struct ht_item **items;
    size_t item_count;
    size_t capacity;

    struct bucket **buckets;
    size_t bucket_count;

    hash_func_t hfp;    // primary hash function
    hash_func_t hfs;    // secondary hash function

    int overflow_flag;
    int collision_flag;
};

#define ht_overflow(ht)         (ht)->overflow_flag = (ht)->item_count + 1 > (ht)->capacity ? 1 : 0
#define ht_colision(ht, index)  (ht)->collision_flag = (ht)->items[index] != NULL ? 1 : 0
#define ht_index(hash, max)     (hash) % (max)

i64 ror(i64 n, int k);

i64 hash_func_primary(const char *s);
i64 hash_func_secondary(const char *s);

void ht_clean(Hash_Table *ht);
void ht_summary(Hash_Table *ht);
void ht_init(Hash_Table *ht, size_t capacity, hash_func_t hf1, hash_func_t hf2);

struct ht_item *ht_item_create(const char *key, i64 value, i64 hash);

struct bucket **buckets_new(size_t count);
struct bucket *bucket_create(struct ht_item *item);
struct bucket *ht_bucket_search(struct bucket *list, i64 hash);
void ht_bucket_push(Hash_Table *ht, struct ht_item *new_item, i64 index);

int ht_get(Hash_Table *ht, const char *key, i64 *dst);
void ht_insert(Hash_Table *ht, const char *key, i64 value);

#endif // HT_H_