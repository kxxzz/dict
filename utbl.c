#include "utbl.h"
#include <vec.h>




typedef struct utbl_Key
{
    u32 off;
    u32 len;
} utbl_Key;


typedef struct utbl_Cell
{
    utbl_Key key;
    uintptr_t val;
} utbl_Cell;

typedef vec_t(utbl_Cell) utbl_CellVec;



typedef struct utbl_Table
{
    vec_u8 keyDataBuf;
    utbl_CellVec cells;
} utbl_Table;





















































































































