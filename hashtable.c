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



enum
{
    HashTable_Seed_Hash = 0,
    HashTable_Seed_Hash1 = 1,
};




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






static u32 calcHashX(u32 keySize, const void* keyData, u32 seed)
{
    u32 hash = XXH32(keyData, keySize, seed);
    return hash;
}





static u32 calcHash(u32 keySize, const void* keyData)
{
    return calcHashX(keySize, keyData, HashTable_Seed_Hash);
}
static u32 calcHash1(u32 keySize, const void* keyData)
{
    return calcHashX(keySize, keyData, HashTable_Seed_Hash1);
}


// https://math.stackexchange.com/questions/2251823/are-all-odd-numbers-coprime-to-powers-of-two
static u32 hashTableCalcShift(HashTable* tbl, u32 keySize, const void* keyData)
{
    u32 shift = calcHash1(keySize, keyData);
    shift = shift % (tbl->slotTable.length - 1) + 1;
    shift += shift % 2 ? 0 : 1;
    return shift;
}


static u32 hashTableNextSlot(HashTable* tbl, u32 si, u32 shift)
{
    si = (si + shift) % tbl->slotTable.length;
    return si;
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
        HashTable_Slot* slot = slotTable0.data + i;
        if (!slot->occupied)
        {
            continue; 
        }
        u32 keySize = slot->key.size;
        const void* keyData = tbl->keyDataBuf.data + slot->key.offset;
        u32 hash = calcHash(keySize, keyData);
        u32 shift = hashTableCalcShift(tbl, keySize, keyData);
        u64 v = slot->val;
        {
            u32 si = hash % tbl->slotTable.length;
            u32 s0 = si;
            for (;;)
            {
                if (!tbl->slotTable.data[si].occupied)
                {
                    *hashTableOccupySlot(tbl, si, hash, keySize, keyData) = v;
                    break;
                }
                si = hashTableNextSlot(tbl, si, shift);
                assert(si != s0);
            }
        }
    }
    vec_free(&slotTable0);   
}







u64* hashTableGet(HashTable* tbl, u32 keySize, const void* keyData)
{
    u32 hash = calcHash(keySize, keyData);
    u32 shift = hashTableCalcShift(tbl, keySize, keyData);
    u32 si = hash % tbl->slotTable.length;
    for (;;)
    {
        if (!tbl->slotTable.data[si].occupied)
        {
            return NULL;
        }
        if (tbl->slotTable.data[si].key.size != keySize)
        {
            goto next;
        }
        const void* keyData0 = tbl->keyDataBuf.data + tbl->slotTable.data[si].key.offset;
        if (memcmp(keyData0, keyData, keySize) != 0)
        {
            goto next;
        }
        return &tbl->slotTable.data[si].val;
    next:
        si = hashTableNextSlot(tbl, si, shift);
    }
}




u64* hashTableAdd(HashTable* tbl, u32 keySize, const void* keyData)
{
    if (tbl->numSlotsUsed > tbl->slotTable.length*0.75f)
    {
        hashTableEnlarge(tbl);
    }
    u32 hash = calcHash(keySize, keyData);
    u32 shift = hashTableCalcShift(tbl, keySize, keyData);
    {
        u32 si = hash % tbl->slotTable.length;
        u32 s0 = si;
        for (;;)
        {
            if (!tbl->slotTable.data[si].occupied)
            {
                return hashTableOccupySlot(tbl, si, hash, keySize, keyData);
            }
            if (tbl->slotTable.data[si].key.size != keySize)
            {
                goto next;
            }
            const void* keyData0 = tbl->keyDataBuf.data + tbl->slotTable.data[si].key.offset;
            if (memcmp(keyData0, keyData, keySize) != 0)
            {
                goto next;
            }
            return &tbl->slotTable.data[si].val;
        next:
            si = hashTableNextSlot(tbl, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
enlarge:
    hashTableEnlarge(tbl);
    {
        u32 si = hash % tbl->slotTable.length;
        u32 s0 = si;
        for (;;)
        {
            if (!tbl->slotTable.data[si].occupied)
            {
                return hashTableOccupySlot(tbl, si, hash, keySize, keyData);
            }
            si = hashTableNextSlot(tbl, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
    goto enlarge;
}










































































































