#ifndef  _INDEXTREE_ITEM_H_
#define  _INDEXTREE_ITEM_H_

#include <string>
#include "indextree/indextree_inner.h"

struct han_item {
	u8 priority     [ BP(1, 1) ];
  	u8 pystr        [1];
};

class HanItem {
public:
    HanItem(void* hi):py(""),priority(0)
    {
        if (hi != NULL) {
            priority = ((struct han_item*)hi)->priority[0];
            py = (char *)(((struct han_item*)hi)->pystr);
        }
    }

    int priority;
    std::string py;
};

struct phrase_item {
	u8 priority     [ BP(1, 1) ];
  	u8 phstr        [1];
};

class PhraseItem {
public:
    PhraseItem(void* pi):ph(""),priority(0)
    {
        if (pi != NULL) {
            priority = ((struct phrase_item*)pi)->priority[0];
            ph = (char *)(((struct phrase_item*)pi)->phstr);
        }
    }

    int priority;
    std::string ph;
};

#endif
