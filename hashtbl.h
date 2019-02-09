#pragma once


#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef float f32;
typedef double f64;





typedef struct hashtbl_t hashtbl_t;

hashtbl_t* hashtbl_new(u32 initSize);
void hashtbl_free(hashtbl_t* tbl);

u64* hashtbl_get(hashtbl_t* tbl, u32 keySize, const void* keyData);
u64* hashtbl_add(hashtbl_t* tbl, u32 keySize, const void* keyData);

static u64* hashtbl_getstr(hashtbl_t* tbl, const char* keyData)
{
    return hashtbl_get(tbl, (u32)strlen(keyData), keyData);
}
static u64* hashtbl_addstr(hashtbl_t* tbl, const char* keyData)
{
    return hashtbl_add(tbl, (u32)strlen(keyData), keyData);
}


































































































