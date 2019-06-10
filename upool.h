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



typedef struct upool* upool_t;

upool_t upoolNew(u32 initSize);
void upoolFree(upool_t pool);



enum
{
    upool_ElmID_NULL = -1,
};

u32 upoolFind(upool_t pool, const void* elmData, u32 elmSize);
u32 upoolElm(upool_t pool, const void* elmData, u32 elmSize, bool* isNew);



const void* upoolElmData(upool_t pool, u32 offset);



u32 upoolElmsTotal(upool_t pool);


typedef void(*upool_ElmCallback)(const void* elmData, u32 elmSize);

void upoolForEach(upool_t pool, upool_ElmCallback cb);




static u32 upoolFindCstr(upool_t pool, const char* elmData)
{
    return upoolFind(pool, elmData, (u32)strlen(elmData));
}
static u32 upoolCstr(upool_t pool, const char* elmData, bool* isNew)
{
    return upoolElm(pool, elmData, (u32)strlen(elmData), isNew);
}





















































































