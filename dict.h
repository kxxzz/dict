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





typedef struct Dict Dict;

Dict* newDict(u32 initSize);
void dictFree(Dict* dict);

u64* dictGet(Dict* dict, u32 keySize, const void* keyData);
u64* dictAdd(Dict* dict, u32 keySize, const void* keyData, bool* isNew);

static u64* dictGetStr(Dict* dict, const char* keyData)
{
    return dictGet(dict, (u32)strlen(keyData), keyData);
}
static u64* dictAddStr(Dict* dict, const char* keyData, bool* isNew)
{
    return dictAdd(dict, (u32)strlen(keyData), keyData, isNew);
}

u32 dictElmsTotal(Dict* dict);


typedef void(*DictElmCallback)(u32 keySize, const void* keyData, u64* value);

void dictForEach(Dict* dict, DictElmCallback cb);



























































































