#include "dict.h"
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
    Dict_Seed_Hash = 0,
    Dict_Seed_Hash1 = 1,
};




typedef struct Dict_Key
{
    u32 offset;
    u32 size;
} Dict_Key;


typedef struct Dict_Slot
{
    bool occupied;
    Dict_Key key;
    u64 val;
} Dict_Slot;

typedef vec_t(Dict_Slot) Dict_SlotVec;



typedef struct Dict
{
    vec_u8 keyDataBuf;
    u32 numSlotsUsed;
    Dict_SlotVec slotTable;
} Dict;

Dict* newDict(u32 initSize)
{
    Dict* tbl = zalloc(sizeof(*tbl));
    vec_resize(&tbl->slotTable, initSize);
    memset(tbl->slotTable.data, 0, initSize * sizeof(*tbl->slotTable.data));
    return tbl;
}

void dictFree(Dict* tbl)
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
    return calcHashX(keySize, keyData, Dict_Seed_Hash);
}
static u32 calcHash1(u32 keySize, const void* keyData)
{
    return calcHashX(keySize, keyData, Dict_Seed_Hash1);
}


// https://math.stackexchange.com/questions/2251823/are-all-odd-numbers-coprime-to-powers-of-two
static u32 dictCalcShift(Dict* tbl, u32 keySize, const void* keyData)
{
    u32 shift = calcHash1(keySize, keyData);
    shift = (shift > 0) ? shift : 1;
    shift = shift % tbl->slotTable.length;
    shift += shift % 2 ? 0 : 1;
    return shift;
}


static u32 dictNextSlot(Dict* tbl, u32 si, u32 shift)
{
    si = (si + shift) % tbl->slotTable.length;
    return si;
}





static u64* dictOccupySlot(Dict* tbl, u32 si, u32 hash, u32 keySize, const void* keyData)
{
    ++tbl->numSlotsUsed;
    u32 offset = tbl->keyDataBuf.length;
    vec_pusharr(&tbl->keyDataBuf, keyData, keySize);
    assert(si < tbl->slotTable.length);
    Dict_Slot* slot = tbl->slotTable.data + si;
    assert(!slot->occupied);
    slot->occupied = true;
    slot->key.offset = offset;
    slot->key.size = keySize;
    return &slot->val;
}


static void dictEnlarge(Dict* tbl)
{
    u32 l0 = tbl->slotTable.length;
    u32 l1 = !l0 ? 1 : l0 << 1;
    Dict_SlotVec slotTable0 = tbl->slotTable;
    memset(&tbl->slotTable, 0, sizeof(tbl->slotTable));
    vec_resize(&tbl->slotTable, l1);
    memset(tbl->slotTable.data, 0, l1 * sizeof(*tbl->slotTable.data));
    for (u32 i = 0; i < slotTable0.length; ++i)
    {
        Dict_Slot* slot = slotTable0.data + i;
        if (!slot->occupied)
        {
            continue; 
        }
        u32 keySize = slot->key.size;
        const void* keyData = tbl->keyDataBuf.data + slot->key.offset;
        u32 hash = calcHash(keySize, keyData);
        u32 shift = dictCalcShift(tbl, keySize, keyData);
        u64 v = slot->val;
        {
            u32 si = hash % tbl->slotTable.length;
            u32 s0 = si;
            for (;;)
            {
                if (!tbl->slotTable.data[si].occupied)
                {
                    *dictOccupySlot(tbl, si, hash, keySize, keyData) = v;
                    break;
                }
                si = dictNextSlot(tbl, si, shift);
                assert(si != s0);
            }
        }
    }
    vec_free(&slotTable0);   
}







u64* dictGet(Dict* tbl, u32 keySize, const void* keyData)
{
    u32 hash = calcHash(keySize, keyData);
    u32 shift = dictCalcShift(tbl, keySize, keyData);
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
        si = dictNextSlot(tbl, si, shift);
    }
}




u64* dictAdd(Dict* tbl, u32 keySize, const void* keyData, bool* isNew)
{
    if (tbl->numSlotsUsed > tbl->slotTable.length*0.75f)
    {
        dictEnlarge(tbl);
    }
    u32 hash = calcHash(keySize, keyData);
    u32 shift = dictCalcShift(tbl, keySize, keyData);
    {
        u32 si = hash % tbl->slotTable.length;
        u32 s0 = si;
        for (;;)
        {
            if (!tbl->slotTable.data[si].occupied)
            {
                if (isNew) *isNew = true;
                return dictOccupySlot(tbl, si, hash, keySize, keyData);
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
            if (isNew) *isNew = false;
            return &tbl->slotTable.data[si].val;
        next:
            si = dictNextSlot(tbl, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
enlarge:
    dictEnlarge(tbl);
    {
        u32 si = hash % tbl->slotTable.length;
        u32 s0 = si;
        for (;;)
        {
            if (!tbl->slotTable.data[si].occupied)
            {
                if (isNew) *isNew = true;
                return dictOccupySlot(tbl, si, hash, keySize, keyData);
            }
            si = dictNextSlot(tbl, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
    goto enlarge;
}







u32 dictElmsTotal(Dict* tbl)
{
    return tbl->numSlotsUsed;
}



void dictForEach(Dict* tbl, DictElmCallback cb)
{
    for (u32 i = 0; i < tbl->slotTable.length; ++i)
    {
        Dict_Slot* slot = tbl->slotTable.data + i;
        if (!slot->occupied)
        {
            continue;
        }
        u32 keySize = slot->key.size;
        const void* keyData = tbl->keyDataBuf.data + slot->key.offset;
        cb(keySize, keyData, &slot->val);
    }
}






























































































