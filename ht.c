#include "ht.h"

const uint32_t sha256_keys[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

const uint32_t md5_shifts[64] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

const uint32_t md5_keys[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

i64 hash_func_primary(const char *str)
{
    i64 result = 0;
    struct md5 m = {0};
    size_t size = strlen(str) - 1;

    m.flag = 0;
    m.h[0] = 0x67452301;
    m.h[1] = 0xefcdab89;
    m.h[2] = 0x98badcfe;
    m.h[3] = 0x10325476; 
    m.chunk_count = size / MD5_CHUNK_SIZE;

    uint32_t i;
    uint32_t temp;
    size_t chunk_i;
    uint32_t *chunk_4b;
    uint32_t a, b, c, d, f, g;
    size_t bits_len = size * 8;

    uint8_t *data = (uint8_t*)str;

    for (chunk_i = 0; chunk_i <= m.chunk_count; chunk_i++) {
        memset(m.chunk, 0, MD5_CHUNK_SIZE);

        if (chunk_i >= m.chunk_count) {
            if (m.flag == 0) {
                m.flag = 1;
                memset(m.sub_chunk, 0, MD5_CHUNK_SIZE * 2);
                memcpy(m.sub_chunk, data + (chunk_i * MD5_CHUNK_SIZE), (size % MD5_CHUNK_SIZE));
                m.sub_chunk[0][size % MD5_CHUNK_SIZE] = 128;

                if ((size % MD5_CHUNK_SIZE) > 55) {
                    memcpy(&m.sub_chunk[1][56], &bits_len, sizeof(bits_len));
                    chunk_i--;
                } else
                    memcpy(&m.sub_chunk[0][56], &bits_len, sizeof(bits_len));
            }
            memcpy(m.chunk, m.sub_chunk[m.flag - 1], MD5_CHUNK_SIZE);
            m.flag++;
        } else
            memcpy(m.chunk, data + (chunk_i * MD5_CHUNK_SIZE), MD5_CHUNK_SIZE);

        chunk_4b = (uint32_t *)m.chunk;

        a = m.h[0];
        b = m.h[1];
        c = m.h[2];
        d = m.h[3];

        for (i = 0; i < 64; i++) {
            switch (i / 16) {
            case 0:
                f = F(b, c, d);
                g = i;
                break;
            case 1:
                f = G(b, c, d);
                g = (5 * i + 1) % 16;
                break;
            case 2:
                f = H(b, c, d);
                g = (3 * i + 5) % 16;
                break;
            case 3:
                f = I(b, c, d);
                g = (7 * i) % 16;
                break;
            }

            temp = d;
            d = c;
            c = b;
            b += rol((a + f + md5_keys[i] + chunk_4b[g]), md5_shifts[i]);
            a = temp;
        }

        m.h[0] += a;
        m.h[1] += b;
        m.h[2] += c;
        m.h[3] += d;
    }

    for (size_t i = 0; i < 4; ++i) {
        result += m.h[i] ^ HT_MAGIC ^ md5_keys[i];
        result <<= 8;
    }

    return result; 
}

void sha256_proc(struct sha256 *s, const uint8_t *buf)
{
    uint32_t W[64], t1, t2, a, b, c, d, e, f, g, h;
    int i;

    for (i = 0; i < 16; i++) {
        W[i] = (uint32_t)buf[4*i]<<24;
        W[i] |= (uint32_t)buf[4*i+1]<<16;
        W[i] |= (uint32_t)buf[4*i+2]<<8;
        W[i] |= buf[4*i+3];
    }

    for (; i < 64; i++)
        W[i] = R1(W[i-2]) + W[i-7] + R0(W[i-15]) + W[i-16];
    
    a = s->h[0];
    b = s->h[1];
    c = s->h[2];
    d = s->h[3];
    e = s->h[4];
    f = s->h[5];
    g = s->h[6];
    h = s->h[7];

    for (i = 0; i < 64; i++) {
        t1 = h + S1(e) + Ch(e, f, g) + sha256_keys[i] + W[i];
        t2 = S0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    s->h[0] += a;
    s->h[1] += b;
    s->h[2] += c;
    s->h[3] += d;
    s->h[4] += e;
    s->h[5] += f;
    s->h[6] += g;
    s->h[7] += h;
}

i64 hash_func_secondary(const char *str)
{
    i64 result = 0;
    struct sha256 s;
    size_t len = strlen(str);
    unsigned char hash[32];

    s.len = 0;
    s.h[0] = 0x6a09e667;
    s.h[1] = 0xbb67ae85;
    s.h[2] = 0x3c6ef372;
    s.h[3] = 0xa54ff53a;
    s.h[4] = 0x510e527f;
    s.h[5] = 0x9b05688c;
    s.h[6] = 0x1f83d9ab;
    s.h[7] = 0x5be0cd19;

    unsigned r = s.len % 64;
    const uint8_t *p = (const uint8_t*)str;
    s.len += len;

    if (r) {
        if (len >= 64 - r) {
            memcpy(s.buf + r, p, 64 - r);
            len -= 64 - r;
            p += 64 - r;
            sha256_proc(&s, s.buf);
        } else
            memcpy(s.buf + r, p, len);
    }

    for (; len >= 64; len -= 64, p += 64)
        sha256_proc(&s, p);

    memcpy(s.buf, p, len);
    unsigned r1 = s.len % 64;
    s.buf[r1++] = 0x80;

    if (r1 > 56) {
        memset(s.buf + r1, 0, 64 - r1);
        sha256_proc(&s, s.buf);
        r1 = 0;
    }

    memset(s.buf + r1, 0, 56 - r1);

    s.len *= 8;
    s.buf[56] = s.len >> 56;
    s.buf[57] = s.len >> 48;
    s.buf[58] = s.len >> 40;
    s.buf[59] = s.len >> 32;
    s.buf[60] = s.len >> 24;
    s.buf[61] = s.len >> 16;
    s.buf[62] = s.len >> 8;
    s.buf[63] = s.len;

    sha256_proc(&s, s.buf);

    for (size_t i = 0; i < 8; i++) {
        hash[4 * i] = s.h[i] >> 24;
        hash[4 * i + 1] = s.h[i] >> 16;
        hash[4 * i + 2] = s.h[i] >> 8;
        hash[4 * i + 3] = s.h[i];
    }

    for (size_t i = 0; i < 32; ++i) {
        result += hash[i] ^ (HT_MAGIC & sha256_keys[i]);
        result <<= 8;
    }

    return result;
}

struct ht_item *ht_item_create(const char *key, void *value, i64 hash)
{
    struct ht_item *item = malloc(sizeof(struct ht_item));
    item->key = key;
    item->value = value;
    item->hash = hash;
    return item;
}

void ht_init(Hash_Table *ht, size_t capacity)
{
    ht->collision_flag = 0;
    ht->overflow_flag = 0;
    ht->bucket_count = 0;
    ht->item_count = 0;
    ht->buckets = NULL;
    ht->capacity = capacity;
    ht->hfp = (*hash_func_primary);
    ht->hfs = (*hash_func_secondary);
    ht->items = malloc(sizeof(struct ht_item) * capacity);
    for (size_t i = 0; i < capacity; ++i) {
        ht->items[i] = NULL;
    }
}

void ht_clean(Hash_Table *ht)
{
    for (size_t i = 0; i < ht->capacity; ++i) {
        free(ht->items[i]);
    }
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

struct bucket **buckets_create(size_t count)
{
    struct bucket **bs = malloc(sizeof(struct bucket) * count);
    for (size_t i = 0; i < count; ++i)
        bs[i] = NULL;
    return bs;
}

void ht_bucket_push(Hash_Table *ht, struct ht_item *new_item, i64 index)
{
    if (ht->buckets == NULL)
        ht->buckets = buckets_create(ht->capacity);

    ht_colision(ht, index);
    if (ht->collision_flag) {
        new_item->hash = ht->hfs(new_item->key); 
        index = new_item->hash % ht->capacity;
    }

    struct bucket *b = bucket_create(new_item);
    b->next = ht->buckets[index];
    ht->buckets[index] = b;
    ht->bucket_count += 1;
}

void ht_insert(Hash_Table *ht, const char *key, void *value)
{
    i64 hash = ht->hfp(key);
    struct ht_item *new_item = ht_item_create(key, value, hash);
    i64 index = ht_index(new_item->hash, ht->capacity);
    
    ht_overflow(ht);
    ht_colision(ht, index);

    if (!ht->overflow_flag) {
        if (ht->collision_flag) {
            new_item->hash = ht->hfs(new_item->key); 
            index = ht_index(new_item->hash, ht->capacity);
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

int ht_get(Hash_Table *ht, const char *key, void **dst)
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
                printf("    hash: [0x%08llx], index: [%lli], key: [%s], value: [%p]\n",
                        item->hash, ht_index(item->hash, ht->capacity), item->key, item->value);
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
                        printf("    hash: [0x%08llx], index: [%lli], key: [%s], value: [%p]\n",
                                b->item->hash, ht_index(b->item->hash, ht->capacity), b->item->key, b->item->value);
                        b = b->next;
                    }
                }
            }
        }
    }
    printf("\n---------------------------------------------------------------------\n\n");
}