#ifndef _IIM_H_
#define _IIM_H_

#include <string>
#include <deque>
// Chinese Simplified language eUniversity Press, 2010.

// Page range [1..n]
#define IM_ITEM_PAGE_SIZE  9
#define ICWIN_W  480
#define ICWIN_H  64

using namespace std;

typedef struct {
    string key;
    string val;
} IMItem;

using namespace std;

class iIM {
public:
    // 'input' : pinyin string of user input.
    // 'items' : candidate target items.
    // 'return': the whole candidate string.
    virtual string lookup(const string& input, deque<IMItem>& items) = 0;
    virtual void addUserPhrase(const string& phrase) = 0;
    virtual void addUserPhraseAsync(const string& phrase) = 0;
    virtual void close() = 0;
    virtual ~iIM() {};
};

#endif
