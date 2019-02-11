#include <stdlib.h>
#ifdef _WIN32
# include <crtdbg.h>
#endif



#include <stdio.h>
#include <assert.h>

#include "hashtable.h"




#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))




static void test(void)
{
    HashTable* tbl = newHashTable(2);
    char* s[] =
    {
        "a", "b", "c", "d", "e"
    };
    char* s1[] =
    {
        "x", "y", "z", "a1", "b1"
    };
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        bool isNew = false;
        uintptr_t* a = hashTableAddStr(tbl, s[i], &isNew);
        *a = (i + 1) * 100;
        assert(isNew);
    }
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        bool isNew = true;
        hashTableAddStr(tbl, s[i], &isNew);
        assert(!isNew);
    }
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        uintptr_t* a = hashTableGetStr(tbl, s[i]);
        assert((i + 1) * 100 == *a);

        uintptr_t* a1 = hashTableGetStr(tbl, s1[i]);
        assert(!a1);
    }
    hashTableFree(tbl);
}





static int mainReturn(int r)
{
#if !defined(NDEBUG) && defined(_WIN32)
    system("pause");
#endif
    return r;
}


int main(int argc, char* argv[])
{
    test();
    mainReturn(0);
}



















































































