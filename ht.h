#ifndef HT_H_
#define HT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define HT_MAGIC    0x428a2f98    // cubic root of 2
#define HT_CAPACITY 69

typedef long long int i64;
typedef i64 (*hash_func_t) (const char *s);

// sha256 implementation stolen from https://github.com/leahneukirchen/redo-c/blob/master/redo.c
struct sha256 {
    uint64_t len;
    uint32_t h[8];
    uint8_t buf[64];
};

extern const uint32_t sha256_keys[64];

#define Ch(x,y,z)  (z ^ (x & (y ^ z)))
#define Maj(x,y,z) ((x & y) | (z & (x | y)))
#define S0(x)      (ror(x,2) ^ ror(x,13) ^ ror(x,22))
#define S1(x)      (ror(x,6) ^ ror(x,11) ^ ror(x,25))
#define R0(x)      (ror(x,7) ^ ror(x,18) ^ (x>>3))
#define R1(x)      (ror(x,17) ^ ror(x,19) ^ (x>>10))

void sha256_proc(struct sha256 *s, const uint8_t *buf);

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

uint32_t ror(uint32_t n, int k);

i64 hash_func_primary(const char *str);
i64 hash_func_secondary(const char *s);

void ht_clean(Hash_Table *ht);
void ht_summary(Hash_Table *ht);
void ht_init(Hash_Table *ht, size_t capacity);

struct ht_item *ht_item_create(const char *key, i64 value, i64 hash);

struct bucket **buckets_create(size_t count);
struct bucket *bucket_create(struct ht_item *item);
struct bucket *ht_bucket_search(struct bucket *list, i64 hash);
void ht_bucket_push(Hash_Table *ht, struct ht_item *new_item, i64 index);

int ht_get(Hash_Table *ht, const char *key, i64 *dst);
void ht_insert(Hash_Table *ht, const char *key, i64 value);

#endif // HT_H_