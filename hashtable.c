#include "hashtable.h"
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




typedef struct HashTable_Key
{
    u32 offset;
    u32 size;
} HashTable_Key;


typedef struct HashTable_Slot
{
    bool occupied;
    HashTable_Key key;
    u64 val;
} HashTable_Slot;

typedef vec_t(HashTable_Slot) HashTable_SlotVec;



typedef struct HashTable
{
    vec_u8 keyDataBuf;
    HashTable_SlotVec slotTable;
} HashTable;

HashTable* newHashTable(u32 initSize)
{
    HashTable* tbl = zalloc(sizeof(*tbl));
    vec_resize(&tbl->slotTable, initSize);
    memset(tbl->slotTable.data, 0, initSize * sizeof(*tbl->slotTable.data));
    return tbl;
}

void hashTableFree(HashTable* tbl)
{
    vec_free(&tbl->slotTable);
    vec_free(&tbl->keyDataBuf);
    free(tbl);
}








static u32 calcHash(u32 keySize, const void* keyData)
{
    u32 seed = 0;
    u32 hash = XXH32(keyData, keySize, seed);
    return hash;
}


static u64* hashTableOccupySlot(HashTable* tbl, u32 si, u32 hash, u32 keySize, const void* keyData)
{
    u32 offset = tbl->keyDataBuf.length;
    vec_pusharr(&tbl->keyDataBuf, keyData, keySize);
    assert(si < tbl->slotTable.length);
    HashTable_Slot* slot = tbl->slotTable.data + si;
    assert(!slot->occupied);
    slot->occupied = true;
    slot->key.offset = offset;
    slot->key.size = keySize;
    return &slot->val;
}


static void hashTableEnlarge(HashTable* tbl)
{
    u32 l0 = tbl->slotTable.length;
    u32 l = !l0 ? 1 : l0 << 1;
    vec_resize(&tbl->slotTable, l);
    memset(tbl->slotTable.data + l0, 0, (l - l0) * sizeof(*tbl->slotTable.data));
}







u64* hashTableGet(HashTable* tbl, u32 keySize, const void* keyData)
{
    u32 hash = calcHash(keySize, keyData);
    for (u32 i = 0; i < tbl->slotTable.length; ++i)
    {
        u32 si = (hash + i) % tbl->slotTable.length;
        if (!tbl->slotTable.data[si].occupied)
        {
            continue;
        }
        if (tbl->slotTable.data[si].key.size != keySize)
        {
            continue;
        }
        const void* keyData0 = tbl->keyDataBuf.data + tbl->slotTable.data[si].key.offset;
        if (0 == memcmp(keyData0, keyData, keySize))
        {
            return &tbl->slotTable.data[si].val;
        }
    }
    return NULL;
}




u64* hashTableAdd(HashTable* tbl, u32 keySize, const void* keyData)
{
    u32 hash = calcHash(keySize, keyData);
    for (u32 i = 0; i < tbl->slotTable.length; ++i)
    {
        u32 si = (hash + i) % tbl->slotTable.length;
        if (!tbl->slotTable.data[si].occupied)
        {
            return hashTableOccupySlot(tbl, si, hash, keySize, keyData);
        }
        if (tbl->slotTable.data[si].key.size != keySize)
        {
            return hashTableOccupySlot(tbl, si, hash, keySize, keyData);
        }
        const void* keyData0 = tbl->keyDataBuf.data + tbl->slotTable.data[si].key.offset;
        if (0 == memcmp(keyData0, keyData, keySize))
        {
            return &tbl->slotTable.data[si].val;
        }
    }
    hashTableEnlarge(tbl);
    for (u32 i = 0; i < tbl->slotTable.length; ++i)
    {
        u32 si = (hash + i) % tbl->slotTable.length;
        if (!tbl->slotTable.data[si].occupied)
        {
            return hashTableOccupySlot(tbl, si, hash, keySize, keyData);
        }
        if (tbl->slotTable.data[si].key.size != keySize)
        {
            return hashTableOccupySlot(tbl, si, hash, keySize, keyData);
        }
    }
    assert(false);
    return NULL;
}










































































































