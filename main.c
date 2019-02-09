#include <stdlib.h>
#ifdef _WIN32
# include <crtdbg.h>
#endif



#include <stdio.h>
#include <assert.h>

#include "hashtbl.h"




#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))




static void test(void)
{
    hashtbl_t* tbl = hashtbl_new(2);
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
        uintptr_t* a = hashtbl_addstr(tbl, s[i]);
        *a = i * 100;
    }
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        uintptr_t* a = hashtbl_getstr(tbl, s[i]);
        assert(i * 100 == *a);

        uintptr_t* a1 = hashtbl_getstr(tbl, s1[i]);
        assert(!a1);
    }
    hashtbl_free(tbl);
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



















































































