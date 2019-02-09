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
    u32 numSlotsUsed;
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
    ++tbl->numSlotsUsed;
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
    u32 l1 = !l0 ? 1 : l0 << 1;
    HashTable_SlotVec slotTable0 = tbl->slotTable;
    memset(&tbl->slotTable, 0, sizeof(tbl->slotTable));
    vec_resize(&tbl->slotTable, l1);
    memset(tbl->slotTable.data, 0, l1 * sizeof(*tbl->slotTable.data));
    for (u32 i = 0; i < slotTable0.length; ++i)
    {
        u32 keySize = slotTable0.data[i].key.size;
        const void* keyData = tbl->keyDataBuf.data + slotTable0.data[i].key.offset;
        u32 hash = calcHash(keySize, keyData);
        u64 v = slotTable0.data[i].val;

        for (u32 i = 0; i < tbl->slotTable.length; ++i)
        {
            u32 si = (hash + i) % tbl->slotTable.length;
            if (!tbl->slotTable.data[si].occupied)
            {
                *hashTableOccupySlot(tbl, si, hash, keySize, keyData) = v;
                break;
            }
        }
    }
    vec_free(&slotTable0);
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
        if (memcmp(keyData0, keyData, keySize) != 0)
        {
            continue;
        }
        return &tbl->slotTable.data[si].val;
    }
    return NULL;
}




u64* hashTableAdd(HashTable* tbl, u32 keySize, const void* keyData)
{
    if (tbl->numSlotsUsed > tbl->slotTable.length*0.75f)
    {
        hashTableEnlarge(tbl);
    }
    u32 hash = calcHash(keySize, keyData);
    for (u32 i = 0; i < tbl->slotTable.length / 2; ++i)
    {
        u32 si = (hash + i) % tbl->slotTable.length;
        if (!tbl->slotTable.data[si].occupied)
        {
            return hashTableOccupySlot(tbl, si, hash, keySize, keyData);
        }
        if (tbl->slotTable.data[si].key.size != keySize)
        {
            continue;
        }
        const void* keyData0 = tbl->keyDataBuf.data + tbl->slotTable.data[si].key.offset;
        if (memcmp(keyData0, keyData, keySize) != 0)
        {
            continue;
        }
        return &tbl->slotTable.data[si].val;
    }
enlarge:
    hashTableEnlarge(tbl);
    for (u32 i = 0; i < tbl->slotTable.length / 2; ++i)
    {
        u32 si = (hash + i) % tbl->slotTable.length;
        if (!tbl->slotTable.data[si].occupied)
        {
            return hashTableOccupySlot(tbl, si, hash, keySize, keyData);
        }
        if (tbl->slotTable.data[si].key.size != keySize)
        {
            continue;
        }
    }
    goto enlarge;
}










































































































