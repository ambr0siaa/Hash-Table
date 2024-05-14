#include "ht.h"

// Stuff for cheking ht's work
// This instuctions gets from my virtual machine
typedef enum {
    INST_MOV = 0,
    INST_MOV_RR,
    INST_MOV_RV,
    INST_MOVS,
    INST_HLT,
    INST_DBR,
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_DIV,
    INST_ADD_RR,
    INST_SUB_RR,
    INST_DIV_RR,
    INST_MUL_RR,
    INST_ADD_RV,
    INST_SUB_RV,
    INST_MUL_RV,
    INST_DIV_RV,
    INST_PUSH,
    INST_PUSH_V,
    INST_PUSH_R,
    INST_POP,
    INST_POP_R,
    INST_POP_N,
    INST_CALL,
    INST_RET,
    INST_RET_N,
    INST_RET_RR,
    INST_RET_RV,
    INST_AND,
    INST_OR,
    INST_NOT,
    INST_XOR,
    INST_SHR,
    INST_SHL,
    INST_AND_RR,
    INST_OR_RR,
    INST_XOR_RR,
    INST_SHR_RR,
    INST_SHL_RR,
    INST_AND_RV,
    INST_OR_RV,
    INST_XOR_RV,
    INST_SHR_RV,
    INST_SHL_RV,
    INST_JMP,
    INST_JNZ,
    INST_JZ,
    INST_CMP,
    INST_VLAD,
    IC
} Inst;

// Convert instruction to string for using as key
char *inst_as_cstr(Inst inst)
{
    switch (inst) {
        case INST_ADD:    return "add";
        case INST_SUB:    return "sub";
        case INST_DIV:    return "div";
        case INST_MUL:    return "mul";
        case INST_ADD_RR: return "addr";
        case INST_SUB_RR: return "subr";
        case INST_DIV_RR: return "divr";
        case INST_MUL_RR: return "mulr";
        case INST_ADD_RV: return "addv";
        case INST_SUB_RV: return "subv";
        case INST_DIV_RV: return "divv";
        case INST_MUL_RV: return "mulv";
        case INST_AND:    return "and";
        case INST_OR:     return "or";
        case INST_NOT:    return "not";
        case INST_XOR:    return "xor";
        case INST_SHR:    return "shr";
        case INST_SHL:    return "shl";
        case INST_AND_RR: return "andr";
        case INST_OR_RR:  return "orr";
        case INST_XOR_RR: return "xorr";
        case INST_SHR_RR: return "shrr";
        case INST_SHL_RR: return "shlr";
        case INST_AND_RV: return "andv";
        case INST_OR_RV:  return "orv";
        case INST_XOR_RV: return "xorv";
        case INST_SHR_RV: return "shrv";
        case INST_SHL_RV: return "shlv";
        case INST_MOV:    return "mov";
        case INST_MOVS:   return "movs";
        case INST_MOV_RR: return "movr";
        case INST_MOV_RV: return "movv";
        case INST_HLT:    return "hlt";
        case INST_DBR:    return "dbr";
        case INST_CMP:    return "cmp";
        case INST_JMP:    return "jmp";
        case INST_JNZ:    return "jnz";
        case INST_JZ:     return "jz";
        case INST_POP:    return "pop";
        case INST_POP_R:  return "popr";
        case INST_POP_N:  return "popn";
        case INST_PUSH:   return "push";
        case INST_PUSH_R: return "pshr";
        case INST_PUSH_V: return "pshv";
        case INST_CALL:   return "call";
        case INST_RET:    return "ret";
        case INST_RET_N:  return "retn";
        case INST_RET_RR: return "retrr";
        case INST_RET_RV: return "retrv";
        case INST_VLAD:   return "vlad";
        case IC:        
        default: {
            fprintf(stderr, "ERROR: `%u` this is not a inst\n", inst);
            exit(1);
        }
    }
}

void generate_ht(Hash_Table *ht)
{
    for (size_t i = INST_MOV; i < IC; ++i) {
        const char *key = inst_as_cstr(i);
        ht_insert(ht, key, i);
    }
}

int main()
{
    Hash_Table ht = {0};
    ht_init(&ht, HT_CAPACITY);
    
    generate_ht(&ht);
    ht_summary(&ht);

    // check that all instructions has put and can be taking out
    for (size_t i = INST_MOV; i < IC; ++i) {
        i64 inst = 0;
        const char *key = inst_as_cstr(i);
        if (!ht_get(&ht, key, &inst)) {
            fprintf(stderr, "ERROR: cannot find inst\n");
            return 1;
        } else {
            printf("inst `%s` has found\n", inst_as_cstr(inst));
        }
    }

    ht_clean(&ht);
    return 0;
}