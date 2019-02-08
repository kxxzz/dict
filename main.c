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
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        uintptr_t* a = hashTableAddStr(tbl, s[i]);
        *a = i * 100;
    }
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        uintptr_t* a = hashTableGetStr(tbl, s[i]);
        assert(i * 100 == *a);
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



















































































