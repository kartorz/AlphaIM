#ifndef _PINYINDB_INNER_H_
#define _PINYINDB_INNER_H_

#include "indextree_inner.h"

#define PYDB_MAGIC_L  0xb3
#define PYDB_MAGIC_H  0xb4

struct pydb_dataitem {
    u8  len_han;
    u8  *ptr_han;
};

#endif
