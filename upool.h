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



enum
{
    Upool_ID_NULL = -1,
};



typedef struct Upool Upool;

Upool* newUpool(u32 initSize);
void upoolFree(Upool* pool);



u32 upoolGet(Upool* pool, u32 elmSize, const void* elmData);
u32 upoolAdd(Upool* pool, u32 elmSize, const void* elmData, bool* isNew);

static u32 upoolGetCstr(Upool* pool, const char* elmData)
{
    return upoolGet(pool, (u32)strlen(elmData), elmData);
}
static u32 upoolAddCstr(Upool* pool, const char* elmData, bool* isNew)
{
    return upoolAdd(pool, (u32)strlen(elmData), elmData, isNew);
}

u32 upoolElmsTotal(Upool* pool);


typedef void(*UpoolElmCallback)(u32 elmSize, const void* elmData);

void upoolForEach(Upool* pool, UpoolElmCallback cb);



























































































