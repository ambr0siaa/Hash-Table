#include "ht.h"

i64 ror(i64 n, int k) { return (n >> k) | (n << (32 - k)); }

i64 hash_func_primary(const char *s)
{
    i64 p = 0x1;
    i64 m = 0xffff;
    i64 hash = 0x0;
    size_t len = strlen(s);
    for (size_t i = 0; i < len; ++i) {
        hash += (i64)(s[i]) * (p ^ HT_MAGIC);
        p = (p << 8) % m;
    }
    return hash;
}

i64 hash_func_secondary(const char *s)
{
    i64 hash1 = 0x0;
    i64 hash2 = 0x0;
    size_t len = strlen(s);
    for (size_t i = 0; i < len; ++i) {
        i64 a = (i64)(s[i]);
        hash1 += ror(a, 7) ^ (ror(a, 11)) ^ ror(a, 17);
        hash2 += (hash1 & hash2) ^ (~hash1 & hash2) ^ k[0];
    }

    hash1 += ((i64)(s[0]) & k[0]) ^ ((i64)(s[len - 1]) & k[1]) ^ (hash1 & k[2]) ^ (hash1 & k[3]);
    hash2 += ((i64)(s[0]) & k[4]) ^ ((i64)(s[len - 1]) & k[5]) ^ (hash2 & k[6]) ^ (hash2 & k[7]);

    return (hash1 | hash2);
}

struct ht_item *ht_item_create(const char *key, i64 value, i64 hash)
{
    struct ht_item *item = malloc(sizeof(struct ht_item));
    item->key = key;
    item->value = value;
    item->hash = hash;
    return item;
}

void ht_init(Hash_Table *ht, size_t capacity, hash_func_t hf1, hash_func_t hf2)
{
    ht->capacity = capacity;
    ht->collision_flag = 0;
    ht->overflow_flag = 0;
    ht->bucket_count = 0;
    ht->item_count = 0;
    ht->buckets = NULL;
    ht->hfp = hf1;
    ht->hfs = hf2;
    ht->items = malloc(sizeof(struct ht_item) * capacity);
    for (size_t i = 0; i < capacity; ++i) {
        ht->items[i] = NULL;
    }
}

void ht_clean(Hash_Table *ht)
{
    for (size_t i = 0; i < ht->capacity; ++i)
        free(ht->items[i]);
    free(ht->items);

    if (ht->overflow_flag ||
        ht->bucket_count > 0) {
        for (size_t i = 0; i < ht->capacity; ++i) {
            struct bucket *b = ht->buckets[i];
            if (b != NULL) {
                while (b != NULL) {
                    struct bucket *next = b->next;
                    free(b->item);
                    free(b);
                    b = next;
                }
            }
        }
        free(ht->buckets);
    }

    ht->overflow_flag = 0;
    ht->collision_flag = 0;
    ht->item_count = 0;
    ht->capacity = 0;
    ht->hfp = NULL;
    ht->hfs = NULL;
}

struct bucket *bucket_create(struct ht_item *item)
{
    struct bucket *bucket = malloc(sizeof(struct bucket));
    bucket->item = item;
    bucket->next = NULL;
    return bucket;
}

struct bucket **buckets_new(size_t count)
{
    struct bucket **bs = malloc(sizeof(struct bucket) * count);
    for (size_t i = 0; i < count; ++i)
        bs[i] = NULL;
    return bs;
}

void ht_bucket_push(Hash_Table *ht, struct ht_item *new_item, i64 index)
{
    if (ht->buckets == NULL)
        ht->buckets = buckets_new(ht->capacity);

    if (ht->buckets[index] != NULL) {
        new_item->hash = ht->hfs(new_item->key); 
        index = new_item->hash % ht->capacity;
    }

    struct bucket *b = bucket_create(new_item);
    b->next = ht->buckets[index];
    ht->buckets[index] = b;
    ht->bucket_count += 1;
}

void ht_insert(Hash_Table *ht, const char *key, i64 value)
{
    i64 hash = ht->hfp(key);
    struct ht_item *new_item = ht_item_create(key, value, hash);
    i64 index = new_item->hash % ht->capacity;
    
    ht_overflow(ht);
    ht_colision(ht, index);

    if (!ht->overflow_flag) {
        if (ht->collision_flag) {
            new_item->hash = ht->hfs(new_item->key); 
            index = new_item->hash % ht->capacity;
            ht_colision(ht, index);

            if (ht->collision_flag) {
                ht_bucket_push(ht, new_item, index);
            } else {
                push:
                ht->items[index] = new_item;
                ht->item_count += 1;
            }
        } else goto push;
    } else ht_bucket_push(ht, new_item, index);
}

struct bucket *ht_bucket_search(struct bucket *list, i64 hash)
{
    struct bucket *b = list;
    while (b != NULL) {
        if (b->item->hash == hash)
            return b;
        b = b->next;
    }
    return NULL;
}

int ht_get(Hash_Table *ht, const char *key, i64 *dst)
{
    i64 hash1 = ht->hfp(key);
    i64 index1 = hash1 % ht->capacity;
    if (ht->items[index1]->hash == hash1) {
        *dst = ht->items[index1]->value;
        return 1;
    }

    i64 hash2 = ht->hfs(key);
    i64 index2 = hash2 % ht->capacity; 
    if (ht->items[index2]->hash == hash2) {
        *dst = ht->items[index2]->value;
        return 1;
    }

    struct bucket *b1 = ht_bucket_search(ht->buckets[index1], hash1);
    if (b1 != NULL) {
        *dst = b1->item->value;
        return 1;
    }

    struct bucket *b2 = ht_bucket_search(ht->buckets[index2], hash2);
    if (b2 != NULL) {
        *dst = b2->item->value;
        return 1;
    }

    return 0;
}

void ht_summary(Hash_Table *ht)
{
    printf("\n------------------------- Hash Table Summary -------------------------\n");
    printf("capacity: [%zu]\n", ht->capacity);
    printf("overflow: [%s]\n", ht->overflow_flag == 0 ? "no" : "yes");
    printf("actual items count: [%zu]\n", ht->item_count);
    if (ht->item_count != 0) {
        for (size_t i = 0; i < ht->capacity; ++i) {
            struct ht_item *item = ht->items[i]; 
            if (item != NULL) {
                printf("    hash: [%lli], index: [%lli], key: [%s], value: [%lli]\n",
                        item->hash, item->hash % ht->capacity, item->key, item->value);
            }
        }
    }
    printf("buckets count: [%zu]\n", ht->bucket_count);
    if (ht->overflow_flag == 1 || ht->bucket_count > 0) {
        if (ht->bucket_count > 0) {
            for (size_t i = 0; i < ht->capacity; ++i) {
                struct bucket *b = ht->buckets[i];
                if (b != NULL) {
                    while (b != NULL) {
                        printf("    hash: [%lli], index: [%lli], key: [%s], value: [%lli]\n",
                                b->item->hash, b->item->hash % ht->capacity, b->item->key, b->item->value);
                        b = b->next;
                    }
                }
            }
        }
    }
    printf("\n---------------------------------------------------------------------\n\n");
}