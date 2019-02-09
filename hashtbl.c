#include "hashtbl.h"
#include <assert.h>
#include <vec.h>
#include <xxhash.h>



#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))




#ifdef max
# undef max
#endif
#ifdef min
# undef min
#endif
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))




#define zalloc(sz) calloc(1, sz)




typedef struct hashtbl_Key
{
    u32 offset;
    u32 size;
} hashtbl_Key;


typedef struct hashtbl_Cell
{
    bool hasVal;
    hashtbl_Key key;
    u64 val;
} hashtbl_Cell;

typedef vec_t(hashtbl_Cell) hashtbl_CellVec;



typedef struct hashtbl_t
{
    vec_u8 keyDataBuf;
    hashtbl_CellVec cellTable;
} hashtbl_t;

hashtbl_t* hashtbl_new(u32 initSize)
{
    hashtbl_t* tbl = zalloc(sizeof(*tbl));
    vec_resize(&tbl->cellTable, initSize);
    memset(tbl->cellTable.data, 0, initSize * sizeof(*tbl->cellTable.data));
    return tbl;
}

void hashtbl_free(hashtbl_t* tbl)
{
    vec_free(&tbl->cellTable);
    vec_free(&tbl->keyDataBuf);
    free(tbl);
}








static u32 calc_hash(u32 keySize, const void* keyData)
{
    u32 seed = 0;
    u32 hash = XXH32(keyData, keySize, seed);
    return hash;
}


static u64* hashtbl_addCell(hashtbl_t* tbl, u32 si, u32 hash, u32 keySize, const void* keyData)
{
    u32 offset = tbl->keyDataBuf.length;
    vec_pusharr(&tbl->keyDataBuf, keyData, keySize);
    assert(si < tbl->cellTable.length);
    hashtbl_Cell* cell = tbl->cellTable.data + si;
    assert(!cell->hasVal);
    cell->hasVal = true;
    cell->key.offset = offset;
    cell->key.size = keySize;
    return &cell->val;
}


static void hashtbl_enlarge(hashtbl_t* tbl)
{
    u32 l0 = tbl->cellTable.length;
    u32 l = !l0 ? 1 : l0 << 1;
    vec_resize(&tbl->cellTable, l);
    memset(tbl->cellTable.data + l0, 0, (l - l0) * sizeof(*tbl->cellTable.data));
}







u64* hashtbl_get(hashtbl_t* tbl, u32 keySize, const void* keyData)
{
    u32 hash = calc_hash(keySize, keyData);
    for (u32 i = 0; i < tbl->cellTable.length; ++i)
    {
        u32 si = (hash + i) % tbl->cellTable.length;
        if (!tbl->cellTable.data[si].hasVal)
        {
            continue;
        }
        if (tbl->cellTable.data[si].key.size != keySize)
        {
            continue;
        }
        const void* keyData0 = tbl->keyDataBuf.data + tbl->cellTable.data[si].key.offset;
        if (0 == memcmp(keyData0, keyData, keySize))
        {
            return &tbl->cellTable.data[si].val;
        }
    }
    return NULL;
}




u64* hashtbl_add(hashtbl_t* tbl, u32 keySize, const void* keyData)
{
    u32 hash = calc_hash(keySize, keyData);
    for (u32 i = 0; i < tbl->cellTable.length; ++i)
    {
        u32 si = (hash + i) % tbl->cellTable.length;
        if (!tbl->cellTable.data[si].hasVal)
        {
            return hashtbl_addCell(tbl, si, hash, keySize, keyData);
        }
        if (tbl->cellTable.data[si].key.size != keySize)
        {
            return hashtbl_addCell(tbl, si, hash, keySize, keyData);
        }
        const void* keyData0 = tbl->keyDataBuf.data + tbl->cellTable.data[si].key.offset;
        if (0 == memcmp(keyData0, keyData, keySize))
        {
            return &tbl->cellTable.data[si].val;
        }
    }
    hashtbl_enlarge(tbl);
    for (u32 i = 0; i < tbl->cellTable.length; ++i)
    {
        u32 si = (hash + i) % tbl->cellTable.length;
        if (!tbl->cellTable.data[si].hasVal)
        {
            return hashtbl_addCell(tbl, si, hash, keySize, keyData);
        }
        if (tbl->cellTable.data[si].key.size != keySize)
        {
            return hashtbl_addCell(tbl, si, hash, keySize, keyData);
        }
    }
    assert(false);
    return NULL;
}










































































































