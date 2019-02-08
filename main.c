#include <stdlib.h>
#ifdef _WIN32
# include <crtdbg.h>
#endif



#include <stdio.h>
#include <assert.h>

#include "utbl.h"




#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))




static void test(void)
{
    UTBL* tbl = UTBL_new(2);
    char* s[] =
    {
        "a", "b", "c", "d", "e"
    };
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        uintptr_t* a = UTBL_addStr(tbl, s[i]);
        *a = i * 100;
    }
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        uintptr_t* a = UTBL_getStr(tbl, s[i]);
        assert(i * 100 == *a);
    }
    UTBL_free(tbl);
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



















































































