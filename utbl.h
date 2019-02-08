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





typedef struct UTBL UTBL;

UTBL* UTBL_new(u32 initSize);
void UTBL_free(UTBL* tbl);

uintptr_t* UTBL_get(UTBL* tbl, u32 keySize, const void* keyData);
uintptr_t* UTBL_add(UTBL* tbl, u32 keySize, const void* keyData);




































































































