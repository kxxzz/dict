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





typedef struct HashTable HashTable;

HashTable* newHashTable(u32 initSize);
void hashTableFree(HashTable* tbl);

uintptr_t* hashTableGet(HashTable* tbl, u32 keySize, const void* keyData);
uintptr_t* hashTableAdd(HashTable* tbl, u32 keySize, const void* keyData);

static uintptr_t* hashTableGetStr(HashTable* tbl, const char* keyData)
{
    return hashTableGet(tbl, (u32)strlen(keyData), keyData);
}
static uintptr_t* hashTableAddStr(HashTable* tbl, const char* keyData)
{
    return hashTableAdd(tbl, (u32)strlen(keyData), keyData);
}


































































































