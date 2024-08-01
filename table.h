#ifndef TABLE_H_
#define TABLE_H_
#define TBDEF extern

#include <stdlib.h>

#define ptr_cast(p) (void*)((p)) // Casting to void pointer for hash-function

typedef unsigned long hash_t;              // Type for hash values
typedef hash_t (*hash_func)(const void *); // Type for hash func
typedef struct table_slot table_slot;      // Table item

struct table_slot {
    hash_t h; // Hash
    int v;    // Value
};

typedef struct {
    hash_func hf;      // Hash-function
    size_t count;      // Amount of elements
    size_t capacity;   // Table size
    table_slot *slots; // Storing slots
} Table;

// Macro intialize table
// It also is universal and indepenent of slots type
#define table_init(t, c, f) \
    do { \
        (t)->slots = malloc((c)*sizeof(*(t)->slots)); \
        memset((t)->slots, 0, (c)*sizeof(*(t)->slots)); \
        (t)->capacity = (c); \
        (t)->count = 0; \
        (t)->hf = (f); \
    } while(0)

TBDEF void table_insert(Table *t, const char *key, int value);
TBDEF int table_search(Table *t, const char *key);

#endif // TABLE_H_
