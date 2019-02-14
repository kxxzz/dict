#include <stdlib.h>
#ifdef _WIN32
# include <crtdbg.h>
#endif



#include <stdio.h>
#include <assert.h>

#include "dict.h"




#ifdef ARYLEN
# undef ARYLEN
#endif
#define ARYLEN(a) (sizeof(a) / sizeof((a)[0]))




static void test(void)
{
    Dict* dict = newDict(2);
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
        uintptr_t* a = dictAddStr(dict, s[i], &isNew);
        *a = (i + 1) * 100;
        assert(isNew);
    }
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        bool isNew = true;
        dictAddStr(dict, s[i], &isNew);
        assert(!isNew);
    }
    for (u32 i = 0; i < ARYLEN(s); ++i)
    {
        uintptr_t* a = dictGetStr(dict, s[i]);
        assert((i + 1) * 100 == *a);

        uintptr_t* a1 = dictGetStr(dict, s1[i]);
        assert(!a1);
    }
    dictFree(dict);
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



















































































