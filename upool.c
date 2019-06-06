#include "upool.h"
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



static u32 align(u32 x, u32 a)
{
    return (x + a - 1) / a * a;
}



enum
{
    Upool_Seed_Hash = 0,
    Upool_Seed_Hash1 = 1,
};




typedef struct Upool_Key
{
    u32 offset;
    u32 size;
} Upool_Key;


typedef struct Upool_Slot
{
    bool occupied;
    Upool_Key elm;
} Upool_Slot;

typedef vec_t(Upool_Slot) Upool_SlotVec;



typedef struct Upool
{
    vec_u8 dataBuf[1];
    u32 numSlotsUsed;
    Upool_SlotVec slotTable[1];
} Upool;

Upool* newUpool(u32 initSize)
{
    Upool* pool = zalloc(sizeof(*pool));
    vec_resize(pool->slotTable, initSize);
    memset(pool->slotTable->data, 0, initSize * sizeof(*pool->slotTable->data));
    return pool;
}

void upoolFree(Upool* pool)
{
    vec_free(pool->slotTable);
    vec_free(pool->dataBuf);
    free(pool);
}






static u32 calcHashX(u32 size, const void* data, u32 seed)
{
    u32 hash = XXH32(data, size, seed);
    return hash;
}





static u32 calcHash(u32 size, const void* data)
{
    return calcHashX(size, data, Upool_Seed_Hash);
}
static u32 calcHash1(u32 elmSize, const void* data)
{
    return calcHashX(elmSize, data, Upool_Seed_Hash1);
}


// https://math.stackexchange.com/questions/2251823/are-all-odd-numbers-coprime-to-powers-of-two
static u32 upoolCalcShift(Upool* pool, const void* data, u32 size)
{
    u32 shift = calcHash1(size, data);
    shift = (shift > 0) ? shift : 1;
    shift = shift % pool->slotTable->length;
    shift += shift % 2 ? 0 : 1;
    return shift;
}


static u32 upoolNextSlot(Upool* pool, u32 si, u32 shift)
{
    si = (si + shift) % pool->slotTable->length;
    return si;
}



static u32 upoolAddData(Upool* pool, const void* elmData, u32 elmSize)
{
    u32 offset = pool->dataBuf->length;
    vec_pusharr(pool->dataBuf, elmData, elmSize);
    u32 a = align(pool->dataBuf->length, sizeof(uintptr_t));
    vec_resize(pool->dataBuf, a);
    return offset;
}

static void upoolOccupySlot(Upool* pool, u32 si, u32 hash, u32 elmSize, u32 elmOffset)
{
    ++pool->numSlotsUsed;
    assert(si < pool->slotTable->length);
    Upool_Slot* slot = pool->slotTable->data + si;
    assert(!slot->occupied);
    slot->occupied = true;
    slot->elm.offset = elmOffset;
    slot->elm.size = elmSize;
}


static void upoolEnlarge(Upool* pool)
{
    pool->numSlotsUsed = 0;
    u32 l0 = pool->slotTable->length;
    u32 l1 = !l0 ? 1 : l0 << 1;
    Upool_SlotVec slotTable0[1] = { pool->slotTable[0] };
    memset(pool->slotTable, 0, sizeof(pool->slotTable));
    vec_resize(pool->slotTable, l1);
    memset(pool->slotTable->data, 0, l1 * sizeof(*pool->slotTable->data));
    for (u32 i = 0; i < slotTable0->length; ++i)
    {
        Upool_Slot* slot = slotTable0->data + i;
        if (!slot->occupied)
        {
            continue; 
        }
        u32 elmSize = slot->elm.size;
        u32 elmOffset = slot->elm.offset;
        const void* elmData = pool->dataBuf->data + elmOffset;
        u32 hash = calcHash(elmSize, elmData);
#ifdef UPOOL_DOUBLEHASHING
        const u32 shift = upoolCalcShift(pool, elmData, elmSize);
#else
        const u32 shift = 1;
#endif
        {
            u32 si = hash % pool->slotTable->length;
            u32 s0 = si;
            for (;;)
            {
                if (!pool->slotTable->data[si].occupied)
                {
                    upoolOccupySlot(pool, si, hash, elmSize, elmOffset);
                    break;
                }
                si = upoolNextSlot(pool, si, shift);
                assert(si != s0);
            }
        }
    }
    vec_free(slotTable0);   
}







u32 upoolFind(Upool* pool, const void* elmData, u32 elmSize)
{
    u32 hash = calcHash(elmSize, elmData);
#ifdef UPOOL_DOUBLEHASHING
    const u32 shift = upoolCalcShift(pool, elmData, elmSize);
#else
    const u32 shift = 1;
#endif
    u32 si = hash % pool->slotTable->length;
    for (;;)
    {
        if (!pool->slotTable->data[si].occupied)
        {
            return Upool_ElmID_NULL;
        }
        if (pool->slotTable->data[si].elm.size != elmSize)
        {
            goto next;
        }
        const void* elmData0 = pool->dataBuf->data + pool->slotTable->data[si].elm.offset;
        if (memcmp(elmData0, elmData, elmSize) != 0)
        {
            goto next;
        }
        return pool->slotTable->data[si].elm.offset;
    next:
        si = upoolNextSlot(pool, si, shift);
    }
}




u32 upoolElm(Upool* pool, const void* elmData, u32 elmSize, bool* isNew)
{
    if (pool->numSlotsUsed > pool->slotTable->length*0.75f)
    {
        upoolEnlarge(pool);
    }
    u32 hash = calcHash(elmSize, elmData);
#ifdef UPOOL_DOUBLEHASHING
    const u32 shift = upoolCalcShift(pool, elmData, elmSize);
#else
    const u32 shift = 1;
#endif
    {
        u32 si = hash % pool->slotTable->length;
        u32 s0 = si;
        for (;;)
        {
            if (!pool->slotTable->data[si].occupied)
            {
                if (isNew) *isNew = true;
                u32 elmOffset = upoolAddData(pool, elmData, elmSize);
                upoolOccupySlot(pool, si, hash, elmSize, elmOffset);
                return elmOffset;
            }
            if (pool->slotTable->data[si].elm.size != elmSize)
            {
                goto next;
            }
            const void* elmData0 = pool->dataBuf->data + pool->slotTable->data[si].elm.offset;
            if (memcmp(elmData0, elmData, elmSize) != 0)
            {
                goto next;
            }
            if (isNew) *isNew = false;
            return pool->slotTable->data[si].elm.offset;
        next:
            si = upoolNextSlot(pool, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
enlarge:
    upoolEnlarge(pool);
    {
        u32 si = hash % pool->slotTable->length;
        u32 s0 = si;
        for (;;)
        {
            if (!pool->slotTable->data[si].occupied)
            {
                if (isNew) *isNew = true;
                u32 elmOffset = upoolAddData(pool, elmData, elmSize);
                upoolOccupySlot(pool, si, hash, elmSize, elmOffset);
                return elmOffset;
            }
            si = upoolNextSlot(pool, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
    goto enlarge;
}






const void* upoolElmData(Upool* pool, u32 offset)
{
    return pool->dataBuf->data + offset;
}







u32 upoolElmsTotal(Upool* pool)
{
    return pool->numSlotsUsed;
}



void upoolForEach(Upool* pool, UpoolElmCallback cb)
{
    for (u32 i = 0; i < pool->slotTable->length; ++i)
    {
        Upool_Slot* slot = pool->slotTable->data + i;
        if (!slot->occupied)
        {
            continue;
        }
        u32 elmSize = slot->elm.size;
        const void* elmData = pool->dataBuf->data + slot->elm.offset;
        cb(elmData, elmSize);
    }
}






























































































