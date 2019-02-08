#include "utbl.h"
#include <vec.h>




typedef struct UTBL_Key
{
    u32 off;
    u32 len;
} UTBL_Key;


typedef struct UTBL_Cell
{
    bool hasVal;
    UTBL_Key key;
    uintptr_t val;
} UTBL_Cell;

typedef vec_t(UTBL_Cell) UTBL_CellVec;



typedef struct UTBL
{
    vec_u8 keyDataBuf;
    UTBL_CellVec cellTable;
} UTBL;





















































































































