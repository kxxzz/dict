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
    upool_Seed_Hash = 0,
    upool_Seed_Hash1 = 1,
};




typedef struct upool_Key
{
    u32 offset;
    u32 size;
} upool_Key;


typedef struct upool_Slot
{
    bool occupied;
    upool_Key elm;
} upool_Slot;

typedef vec_t(upool_Slot) upool_SlotVec;



struct upool
{
    vec_u8 dataBuf[1];
    u32 numSlotsUsed;
    upool_SlotVec slotTable[1];
};

upool_t upool_new(u32 initSize)
{
    upool_t pool = zalloc(sizeof(*pool));
    vec_resize(pool->slotTable, initSize);
    memset(pool->slotTable->data, 0, initSize * sizeof(*pool->slotTable->data));
    return pool;
}

void upool_free(upool_t pool)
{
    vec_free(pool->slotTable);
    vec_free(pool->dataBuf);
    free(pool);
}






static u32 calc_hashX(u32 size, const void* data, u32 seed)
{
    u32 hash = XXH32(data, size, seed);
    return hash;
}





static u32 calc_hash(u32 size, const void* data)
{
    return calc_hashX(size, data, upool_Seed_Hash);
}
static u32 calc_hash1(u32 elmSize, const void* data)
{
    return calc_hashX(elmSize, data, upool_Seed_Hash1);
}


// https://math.stackexchange.com/questions/2251823/are-all-odd-numbers-coprime-to-powers-of-two
static u32 upool_calcShift(upool_t pool, const void* data, u32 size)
{
    u32 shift = calc_hash1(size, data);
    shift = (shift > 0) ? shift : 1;
    shift = shift % pool->slotTable->length;
    shift += shift % 2 ? 0 : 1;
    return shift;
}


static u32 upool_nextSlot(upool_t pool, u32 si, u32 shift)
{
    si = (si + shift) % pool->slotTable->length;
    return si;
}



static u32 upool_addData(upool_t pool, const void* elmData, u32 elmSize)
{
    u32 offset = pool->dataBuf->length;
    vec_pusharr(pool->dataBuf, elmData, elmSize);
    u32 a = align(pool->dataBuf->length, sizeof(uintptr_t));
    vec_resize(pool->dataBuf, a);
    return offset;
}

static void upool_occupySlot(upool_t pool, u32 si, u32 hash, u32 elmSize, u32 elmOffset)
{
    ++pool->numSlotsUsed;
    assert(si < pool->slotTable->length);
    upool_Slot* slot = pool->slotTable->data + si;
    assert(!slot->occupied);
    slot->occupied = true;
    slot->elm.offset = elmOffset;
    slot->elm.size = elmSize;
}


static void upool_enlarge(upool_t pool)
{
    pool->numSlotsUsed = 0;
    u32 l0 = pool->slotTable->length;
    u32 l1 = !l0 ? 1 : l0 << 1;
    upool_SlotVec slotTable0[1] = { pool->slotTable[0] };
    memset(pool->slotTable, 0, sizeof(pool->slotTable));
    vec_resize(pool->slotTable, l1);
    memset(pool->slotTable->data, 0, l1 * sizeof(*pool->slotTable->data));
    for (u32 i = 0; i < slotTable0->length; ++i)
    {
        upool_Slot* slot = slotTable0->data + i;
        if (!slot->occupied)
        {
            continue; 
        }
        u32 elmSize = slot->elm.size;
        u32 elmOffset = slot->elm.offset;
        const void* elmData = pool->dataBuf->data + elmOffset;
        u32 hash = calc_hash(elmSize, elmData);
#ifdef UPOOL_DOUBLEHASHING
        const u32 shift = upool_calcShift(pool, elmData, elmSize);
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
                    upool_occupySlot(pool, si, hash, elmSize, elmOffset);
                    break;
                }
                si = upool_nextSlot(pool, si, shift);
                assert(si != s0);
            }
        }
    }
    vec_free(slotTable0);   
}







u32 upool_find(upool_t pool, const void* elmData, u32 elmSize)
{
    u32 hash = calc_hash(elmSize, elmData);
#ifdef UPOOL_DOUBLEHASHING
    const u32 shift = upool_calcShift(pool, elmData, elmSize);
#else
    const u32 shift = 1;
#endif
    u32 si = hash % pool->slotTable->length;
    for (;;)
    {
        if (!pool->slotTable->data[si].occupied)
        {
            return upool_ElmID_NULL;
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
        si = upool_nextSlot(pool, si, shift);
    }
}




u32 upool_elm(upool_t pool, const void* elmData, u32 elmSize, bool* isNew)
{
    if (pool->numSlotsUsed > pool->slotTable->length*0.75f)
    {
        upool_enlarge(pool);
    }
    u32 hash = calc_hash(elmSize, elmData);
#ifdef UPOOL_DOUBLEHASHING
    const u32 shift = upool_calcShift(pool, elmData, elmSize);
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
                u32 elmOffset = upool_addData(pool, elmData, elmSize);
                upool_occupySlot(pool, si, hash, elmSize, elmOffset);
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
            si = upool_nextSlot(pool, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
enlarge:
    upool_enlarge(pool);
    {
        u32 si = hash % pool->slotTable->length;
        u32 s0 = si;
        for (;;)
        {
            if (!pool->slotTable->data[si].occupied)
            {
                if (isNew) *isNew = true;
                u32 elmOffset = upool_addData(pool, elmData, elmSize);
                upool_occupySlot(pool, si, hash, elmSize, elmOffset);
                return elmOffset;
            }
            si = upool_nextSlot(pool, si, shift);
            if (si == s0)
            {
                break;
            }
        }
    }
    goto enlarge;
}






const void* upool_elmData(upool_t pool, u32 offset)
{
    return pool->dataBuf->data + offset;
}







u32 upool_elmsTotal(upool_t pool)
{
    return pool->numSlotsUsed;
}



void upool_forEach(upool_t pool, upool_ElmCallback cb)
{
    for (u32 i = 0; i < pool->slotTable->length; ++i)
    {
        upool_Slot* slot = pool->slotTable->data + i;
        if (!slot->occupied)
        {
            continue;
        }
        u32 elmSize = slot->elm.size;
        const void* elmData = pool->dataBuf->data + slot->elm.offset;
        cb(elmData, elmSize);
    }
}






























































































