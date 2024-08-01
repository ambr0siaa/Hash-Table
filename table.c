#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define BIL_IMPLEMENTATION
#include "bil.h"

typedef unsigned long u64;
typedef u64 hash_t;

typedef hash_t (*hash_func)(char *, size_t);
typedef double (*test_func)(double);

struct table_slot{
    int v;
    hash_t h;
    Bil_String_View k;
};

#define TABLE_SCALER 3

// Field `scaler` used when table will overflow
// Increments to some power of 2
// Value for incrementing maybe diffrent. It's dependes of using cases
typedef struct {
    hash_func hf;
    size_t scaler;
    size_t count;
    size_t capacity;
    struct table_slot *slots;
} Table;

typedef struct {
    u64 start_value;
    size_t startf, startp;
    size_t collisions;
    size_t keysf;
    double time;
} best_config;

#define LIM 128
#define TEST_COUNT 4

static size_t current_collisions = 0;
static u64 ps[LIM] = {0};
static size_t ptr = 0;

static const u64 tp[LIM] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53,
    59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
    191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251,
    257, 263, 269, 271, 277, 281, 283, 293, 307, 311, 313, 317,
    331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397,
    401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
    467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557,
    563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619,
    631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701,
    709, 719
};

static u64 tk[LIM] = {
    0x65992, 0xf1b28, 0x4540e, 0xb80ee, 0x1149, 0xdd8b6, 0x432db, 0xf1620,
    0x82161, 0xb6a19, 0xdf536, 0xbade6, 0xf10cb, 0x87869, 0xf244f, 0xe030a,
    0xbc408, 0x3f035, 0x7e689, 0x4b71e, 0xb3bc0, 0xdabe2, 0x3cec4, 0x7c8e1,
    0xe1ddb, 0xd9c64, 0xbef96, 0xefefb, 0x8cdea, 0xf2fc2, 0x38ba7, 0x8ea10,
    0x51955, 0xaf48c, 0x369fc, 0xef19a, 0xf35de, 0xe43f4, 0xd6c48, 0xeea76,
    0xf3878, 0x55a4d, 0xc43d1, 0x32674, 0x93d6f, 0x73249, 0xd4adc, 0xf3cbf,
    0xa928a, 0xe677a, 0xd39c0, 0xed341, 0x97418, 0xe72bd, 0xc8069, 0x98f23,
    0x5daee, 0xa6041, 0xd16bb, 0x29ea7, 0xec24a, 0xa46cc, 0x9c4a7, 0xf41a3,
    0x61a8c, 0xe92ca, 0x678d7, 0xa1346, 0x2383e, 0xea6a1, 0x65977, 0x9f934,
    0xce05e, 0xa1373, 0x6790e, 0xeb036, 0xcf2ca, 0x61a55, 0x6984d, 0xca76c,
    0x5fa95, 0xf40de, 0xc93fa, 0xd16da, 0xa606d, 0xf3fcc, 0xecafa, 0x18cb3,
    0xa79ac, 0x5bab9, 0x6f53b, 0x16a4a, 0xf3cbc, 0x958ac, 0xd4afa, 0xe5bd8,
    0xf3abf, 0xd5bd2, 0x12562, 0xc2efb, 0xe43df, 0x905d9, 0xc1a0b, 0xe3774,
    0xaf4b6, 0x78d4c, 0xd8cbc, 0xb0caa, 0x7ab44, 0x8cdb8, 0xbef71, 0xe1dc4,
    0x4d7bb, 0xf2c39, 0xdabfd, 0xf2867, 0x7e6bd, 0xf0b32, 0xdbb35, 0xb5332,
    0x495f7, 0xbadbf, 0x33a0, 0x474f1, 0xf1629, 0xde701, 0x83ec1, 0xde733
};

enum {
    FUNC_SIN = 0,
    FUNC_COS,
    FUNC_SQRT,
    FUNC_CBRT
};

test_func funcs[TEST_COUNT] = {
    [FUNC_SIN] = sin,
    [FUNC_COS] = cos,
    [FUNC_SQRT] = sqrt,
    [FUNC_CBRT] = cbrt
};

char *func_as_cstr(int code)
{
    switch (code) {
        case FUNC_SIN:  return "sin";
        case FUNC_COS:  return "cos";
        case FUNC_SQRT: return "sqrt";
        case FUNC_CBRT: return "cbrt";
        default:
            assert(0 && "Unreachable");
    }
}

#define slot_new(key, val) (struct table_slot) { .k = (key), .v = (val) }
#define table_overflow(t) (((t)->count + 1) >= (3 * (t)->capacity / 4))

#define table_init(t, c, f) \
    do { \
        (t)->slots = malloc((c)*sizeof(*(t)->slots)); \
        if ((t)->slots) { \
            memset((t)->slots, 0, (c)*sizeof(*(t)->slots)); \
            (t)->capacity = (c); \
            (t)->scaler = 1; \
            (t)->count = 0; \
            (t)->hf = (f); \
        } \
    } while(0)

#define table_clean(t) \
    do { \
        free((t)->slots); \
        (t)->slots = NULL; \
        (t)->hf = NULL; \
        (t)->capacity = 0; \
        (t)->scaler = 0; \
        (t)->count = 0; \
    } while(0)

#define index_step(i) ((size_t)(sqrt((double)(i))*1e6))

bool table_insert_item(Table *t, struct table_slot item, size_t index)
{
    bool inserted = false;
    struct table_slot slot = t->slots[index];

    if (slot.h == 0) { // Free slot
        t->slots[index] = item;
        t->count += 1;
        inserted = true;

    } else if (slot.h == item.h) { // In this case increment value
        t->slots[index].v += 1;
        inserted = true;

    } else { // Collision
        current_collisions += 1; // Used for debug information

        for (size_t i = 0; i < t->capacity; ++i) {
            index = (index + index_step(index)) % t->capacity;
            if (t->slots[index].h == 0) {
                t->slots[index] = item;
                t->count += 1;
                inserted = true;
                break;
            }
        }
    }

    return inserted;
}

#define table_resize { table_clean(&new); t->scaler *= 2; goto overflow; }

void table_insert(Table *t, struct table_slot item)
{
    bool inserted;

    item.h = t->hf(item.k.data, item.k.count);
    size_t index = item.h % t->capacity;

    if (table_overflow(t)) goto overflow;

    inserted = table_insert_item(t, item, index);

    if (!inserted) { // Overflow!
    overflow:
        Table new = {0};
        table_init(&new, t->capacity * t->scaler, t->hf);
        if (!new.slots) {
            t->capacity = t->count * 4;
            goto overflow;
        }

        new.scaler = t->scaler << TABLE_SCALER;

        // Copy all table to new
        for (size_t i = 0; i < t->capacity; ++i) {
            struct table_slot slot = t->slots[i];
            if (slot.h != 0) {
                index = slot.h % new.capacity;
                if (!table_insert_item(&new, slot, index))
                    table_resize;
            }
        }

        index = item.h % new.capacity;
        inserted = table_insert_item(&new, item, index);
        if (!inserted) table_resize;

        table_clean(t);
        *t = new;
    }
}

u64 ror(u64 n, u64 s) { return (n << s | n >> (64 - s)); }
u64 rol(u64 n, u64 s) { return (n >> s | n << (64 - s)); }

hash_t djb2_hash(char *s, size_t len) // djb2 hash-function
{
    hash_t result = 5381;
    for (size_t i = 0; i < len; ++i)
        result = result * 33 + (hash_t)s[i];

    return result;
}


hash_t hash_function(char *s, size_t len)
{
    hash_t h = ps[ptr];

    for (size_t i = 0; i < len; ++i) {
        u64 a = (u64)s[i] * tp[(u64)s[i]];
        u64 b = (a * (tk[(a + 1) % LIM] ^ tk[(a - 1) % LIM]));

        h = rol(h * b, tp[(a + 1) % LIM] % 64);
        h ^= tk[a % LIM];
        h = ror(h ^ tk[b % LIM], tp[(a - 1) % LIM] % 64);
    }

    return h;
}

hash_t hashfunc_main(char *s, size_t len)
{
    hash_t h = 0x9d200; // sine of 271 

    for (size_t i = 0; i < len; ++i) {
        u64 a = (u64)s[i] * tp[(u64)s[i]];
        hash_t b = (a * (tk[(a + 1) % LIM] ^ tk[(a - 1) % LIM]));

        h = rol(h * b, tp[(a + 1) % LIM] % 64);
        h ^= tk[a % LIM];
        h = ror(h ^ tk[b % LIM], tp[(a - 1) % LIM] % 64);
    }

    return h;
}

void create_keys(test_func f)
{
    for (size_t i = 0; i < LIM; ++i) {
        double a = f(tp[i]);
        a = a < 0 ? (-1) * a : a;
        u64 b = (u64)(a * 1e6);
        tk[i] = b;
    }
}

void create_ps(test_func f)
{
    for (size_t i = 0; i < LIM; ++i) {
        double a = f(tp[i]);
        a = a < 0 ? (-1) * a : a;
        u64 b = (u64)(a * 1e6);
        ps[i] = b;
    }
}

Bil_String_View split_by_word(Bil_String_View *content)
{
    size_t i = 0;
    Bil_String_View word = {0};
    Bil_String_View separators = bil_sv_from_cstr(".,*<>()'?!;:\"-[]");

    bil_sv_cut_space_left(content);
    while (i < content->count && !isspace(content->data[i])) {
        if (bil_char_in_sv(separators, content->data[i])) break;
        i++;
    }

    i = i == 0 ? 1 : i;

    word.data = content->data;
    word.count = i;

    content->data += i;
    content->count -= i;

    bil_sv_cut_space_left(content);
    return word;
}

void hashfunc_analysis(Bil_String_View content)
{
    size_t collisions = 0;
    double creating_time = 0;
    best_config best = { .collisions = 99999999 };

    for (size_t n = 0; n < TEST_COUNT; ++n) {
        for (; ptr < LIM; ++ptr) {
            create_ps(funcs[n]);

            for (size_t i = 0; i < TEST_COUNT; ++i) {
                Table t = {0};
                Bil_String_View copy = content;
                struct timeval start, end;

                create_keys(funcs[i]);
                table_init(&t, 1000, (*hash_function));

                gettimeofday(&start, NULL);

                for (; content.count > 0;) {
                    Bil_String_View word = split_by_word(&content);
                    table_insert(&t, slot_new(word, 0));
                }

                gettimeofday(&end, NULL);

                creating_time = workflow_time_taken(end, start);
                collisions = current_collisions;

                if (best.collisions > collisions) {
                    best.collisions = collisions;
                    best.time = creating_time;
                    best.start_value = ps[ptr];
                    best.startp = tp[ptr];
                    best.startf = n;
                    best.keysf = i;
                }

                current_collisions = 0;
                table_clean(&t);
                content = copy;
            }
        }

        ptr = 0;
    }

    bil_log(BIL_INFO, "  Best setup:");
    bil_log(BIL_INFO, "     Common :: collisions: %zu, time: %lfs, p: %llu", best.collisions, best.time, best.start_value);
    bil_log(BIL_INFO, "     Start  :: func: %s, prime: %zu", func_as_cstr(best.startf), best.startp);
    bil_log(BIL_INFO, "     Keys   :: func: %s\n", func_as_cstr(best.keysf));
}

// Used for generating keys for array `tk`
void keys_print(void)
{
    for (size_t i = 0; i < LIM; ++i) {
        double a = funcs[FUNC_COS](tp[i]);
        a = a < 0 ? (-1) * a : a;
        u64 b = (u64)(a * 1e6);
        if (i != 0 && i % 8 == 0) printf("\n");
        printf("0x%lx", b);
        if (i + 1 != LIM) printf(", ");
    }

    printf("\n");
}

// Used for generating start value for hash
void startp_print(void)
{
    double a = funcs[FUNC_SIN](673);
    u64 b = (u64)(a * 1e6);
    printf("0x%lx\n", b);
}

double hash_perfomance(size_t count)
{
    double result = (double)(current_collisions - count) / (double)(current_collisions) * 100;
    
    if (result > 100) {
        result = (double)(count - current_collisions) / (double)(count) * 100;
    } else  {
        result *= -1;
    }

    return result;
}

void test_table(Bil_String_View content)
{
    Table t = {0};
    table_init(&t, 1000, *hashfunc_main);

    while (content.count > 0) {
        Bil_String_View word = split_by_word(&content);
        table_insert(&t, slot_new(word, 0));
    }
    
    bil_log(BIL_INFO, "    Detected %zu collision from %zu elements and %zu capacity", current_collisions, t.count, t.capacity);
    bil_log(BIL_INFO, "    Perfomance is %.2lf percent", hash_perfomance(t.count));
    table_clean(&t);
}

// TODO: sort table by values and print top 10 word
int main(void)
{
    const char *file_path = "t8.shakespeare.txt";
    Bil_String_View content = {0};

bil_workflow_begin();

    if (!bil_read_entire_file(file_path, &content)) return 1;
    //hashfunc_analysis(content);
    test_table(content);
    
bil_workflow_end();

    return 0;
}
