#include <stdlib.h>
#ifdef _WIN32
# include <crtdbg.h>
#endif



#include <stdio.h>
#include <assert.h>

#include "upool.h"




#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))




static void test(void)
{
    Upool* pool = newUpool(2);
    char* s[] =
    {
        "a", "b", "c", "d", "e"
    };
    char* s1[] =
    {
        "x", "y", "z", "a1", "b1"
    };
    u32 ids[ARYLEN(s)] = { 0 };
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        bool isNew = false;
        u32 id = upoolAddCstr(pool, s[i], &isNew);
        ids[i] = id;
        assert(isNew);
    }
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        bool isNew = true;
        u32 id = upoolAddCstr(pool, s[i], &isNew);
        assert(id == ids[i]);
        assert(!isNew);
    }
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        u32 id = upoolGetCstr(pool, s[i]);
        assert(id == ids[i]);

        u32 id1 = upoolGetCstr(pool, s1[i]);
        assert(-1 == id1);
    }
    upoolFree(pool);
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



















































































