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

u64* hashTableGet(HashTable* tbl, u32 keySize, const void* keyData);
u64* hashTableAdd(HashTable* tbl, u32 keySize, const void* keyData, bool* isNew);

static u64* hashTableGetStr(HashTable* tbl, const char* keyData)
{
    return hashTableGet(tbl, (u32)strlen(keyData), keyData);
}
static u64* hashTableAddStr(HashTable* tbl, const char* keyData, bool* isNew)
{
    return hashTableAdd(tbl, (u32)strlen(keyData), keyData, isNew);
}

u32 hashTableElmsTotal(HashTable* tbl);


typedef void(*HashTableElmCallback)(u32 keySize, const void* keyData, u64* value);

void hashTableForEach(HashTable* tbl, HashTableElmCallback cb);



























































































