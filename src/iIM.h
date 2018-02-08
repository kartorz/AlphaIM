#ifndef _IIM_H_
#define _IIM_H_

#include <string>
#include <deque>
// Chinese Simplified language eUniversity Press, 2010.

// Page range [1..n]
#define IM_ITEM_PAGE_SIZE  9
#define IM_INPUT_MAX  40


using namespace std;

typedef struct {
    string key;
    string val;
    int    priority;

    // Temp value for lookup..
    int  off;
    int  number;
} IMItem;

class iIM {
public:
    // 'input' : pinyin string of user input.
    // 'items' : candidate target items.
    // 'return': the whole candidate string.
    virtual string lookup(const string& input, deque<IMItem>& items) = 0;

    //
    virtual void onCommit(const IMItem& imitem) = 0;

    // Add phrase to user db.
    virtual void addUserPhrase(const string& phrase) = 0;
    virtual void addUserPhraseAsync(const string& phrase) = 0;

    // Disable IC.
    virtual void close() = 0;
    
    // Change IC focus.
    virtual void reset() = 0;

    virtual ~iIM() {};
};

#endif
