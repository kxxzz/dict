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

upool_t upool_new(u32 initSize);
void upool_free(upool_t pool);



enum
{
    upool_ElmID_NULL = -1,
};

u32 upool_find(upool_t pool, const void* elmData, u32 elmSize);
u32 upool_elm(upool_t pool, const void* elmData, u32 elmSize, bool* isNew);



const void* upool_elmData(upool_t pool, u32 offset);



u32 upool_elmsTotal(upool_t pool);


typedef void(*upool_ElmCallback)(const void* elmData, u32 elmSize);

void upool_forEach(upool_t pool, upool_ElmCallback cb);




static u32 upool_findCstr(upool_t pool, const char* elmData)
{
    return upool_find(pool, elmData, (u32)strlen(elmData));
}
static u32 upool_cstr(upool_t pool, const char* elmData, bool* isNew)
{
    return upool_elm(pool, elmData, (u32)strlen(elmData), isNew);
}





















































































