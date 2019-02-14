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
    Dict* dict = zalloc(sizeof(*dict));
    vec_resize(&dict->slotTable, initSize);
    memset(dict->slotTable.data, 0, initSize * sizeof(*dict->slotTable.data));
    return dict;
}

void dictFree(Dict* dict)
{
    vec_free(&dict->slotTable);
    vec_free(&dict->keyDataBuf);
    free(dict);
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
static u32 dictCalcShift(Dict* dict, u32 keySize, const void* keyData)
{
    u32 shift = calcHash1(keySize, keyData);
    shift = (shift > 0) ? shift : 1;
    shift = shift % dict->slotTable.length;
    shift += shift % 2 ? 0 : 1;
    return shift;
}


static u32 dictNextSlot(Dict* dict, u32 si, u32 shift)
{
    si = (si + shift) % dict->slotTable.length;
    return si;
}





static u64* dictOccupySlot(Dict* dict, u32 si, u32 hash, u32 keySize, const void* keyData)
{
    ++dict->numSlotsUsed;
    u32 offset = dict->keyDataBuf.length;
    vec_pusharr(&dict->keyDataBuf, keyData, keySize);
    assert(si < dict->slotTable.length);
    Dict_Slot* slot = dict->slotTable.data + si;
    assert(!slot->occupied);
    slot->occupied = true;
    slot->key.offset = offset;
    slot->key.size = keySize;
    return &slot->val;
}


static void dictEnlarge(Dict* dict)
{
    u32 l0 = dict->slotTable.length;
    u32 l1 = !l0 ? 1 : l0 << 1;
    Dict_SlotVec slotTable0 = dict->slotTable;
    memset(&dict->slotTable, 0, sizeof(dict->slotTable));
    vec_resize(&dict->slotTable, l1);
    memset(dict->slotTable.data, 0, l1 * sizeof(*dict->slotTable.data));
    for (u32 i = 0; i < slotTable0.length; ++i)
    {
        Dict_Slot* slot = slotTable0.data + i;
        if (!slot->occupied)
        {
            continue; 
        }
        u32 keySize = slot->key.size;
        const void* keyData = dict->keyDataBuf.data + slot->key.offset;
        u32 hash = calcHash(keySize, keyData);
        u32 shift = dictCalcShift(dict, keySize, keyData);
        u64 v = slot->val;
        {
            u32 si = hash % dict->slotTable.length;
            u32 s0 = si;
            for (;;)
            {
                if (!dict->slotTable.data[si].occupied)
                {
                    *dictOccupySlot(dict, si, hash, keySize, keyData) = v;
                    break;
                }
                si = dictNextSlot(dict, si, shift);
                assert(si != s0);
            }
        }
    }
    vec_free(&slotTable0);   
}







u64* dictGet(Dict* dict, u32 keySize, const void* keyData)
{
    u32 hash = calcHash(keySize, keyData);
    u32 shift = dictCalcShift(dict, keySize, keyData);
    u32 si = hash % dict->slotTable.length;
    for (;;)
    {
        if (!dict->slotTable.data[si].occupied)
        {
            return NULL;
        }
        if (dict->slotTable.data[si].key.size != keySize)
        {
            goto next;
        }
        const void* keyData0 = dict->keyDataBuf.data + dict->slotTable.data[si].key.offset;
        if (memcmp(keyData0, keyData, keySize) != 0)
        {
            goto next;
        }
        return &dict->slotTable.data[si].val;
    next:
        si = dictNextSlot(dict, si, shift);
    }
}




u64* dictAdd(Dict* dict, u32 keySize, const void* keyData, bool* isNew)
{
    if (dict->numSlotsUsed > dict->slotTable.length*0.75f)
    {
        dictEnlarge(dict);
    }
    u32 hash = calcHash(keySize, keyData);
    u32 shift = dictCalcShift(dict, keySize, keyData);
    {
        u32 si = hash % dict->slotTable.length;
        u32 s0 = si;
        for (;;)
        {
            if (!dict->slotTable.data[si].occupied)
            {
                if (isNew) *isNew = true;
                return dictOccupySlot(dict, si, hash, keySize, keyData);
            }
            if (dict->slotTable.data[si].key.size != keySize)
            {
                goto next;
            }
            const void* keyData0 = dict->keyDataBuf.data + dict->slotTable.data[si].key.offset;
            if (memcmp(keyData0, keyData, keySize) != 0)
            {
                goto next;
            }
            if (isNew) *isNew = false;
            return &dict->slotTable.data[si].val;
        next:
            si = dictNextSlot(dict, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
enlarge:
    dictEnlarge(dict);
    {
        u32 si = hash % dict->slotTable.length;
        u32 s0 = si;
        for (;;)
        {
            if (!dict->slotTable.data[si].occupied)
            {
                if (isNew) *isNew = true;
                return dictOccupySlot(dict, si, hash, keySize, keyData);
            }
            si = dictNextSlot(dict, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
    goto enlarge;
}







u32 dictElmsTotal(Dict* dict)
{
    return dict->numSlotsUsed;
}



void dictForEach(Dict* dict, DictElmCallback cb)
{
    for (u32 i = 0; i < dict->slotTable.length; ++i)
    {
        Dict_Slot* slot = dict->slotTable.data + i;
        if (!slot->occupied)
        {
            continue;
        }
        u32 keySize = slot->key.size;
        const void* keyData = dict->keyDataBuf.data + slot->key.offset;
        cb(keySize, keyData, &slot->val);
    }
}






























































































