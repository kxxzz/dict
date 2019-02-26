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



typedef struct Upool Upool;

Upool* newUpool(u32 initSize);
void upoolFree(Upool* pool);



enum
{
    Upool_ElmID_NULL = -1,
};

u32 upoolGetElm(Upool* pool, const void* elmData, u32 elmSize);
u32 upoolAddElm(Upool* pool, const void* elmData, u32 elmSize, bool* isNew);



const void* upoolElmData(Upool* pool, u32 id);



u32 upoolElmsTotal(Upool* pool);


typedef void(*UpoolElmCallback)(const void* elmData, u32 elmSize);

void upoolForEach(Upool* pool, UpoolElmCallback cb);




static u32 upoolGetCstr(Upool* pool, const char* elmData)
{
    return upoolGetElm(pool, elmData, (u32)strlen(elmData));
}
static u32 upoolAddCstr(Upool* pool, const char* elmData, bool* isNew)
{
    return upoolAddElm(pool, elmData, (u32)strlen(elmData), isNew);
}





















































































