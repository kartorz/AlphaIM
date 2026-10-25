#ifndef _USRPHRASE_H_
#define _USRPHRASE_H_

#include <set>
#include <string>
#include <vector>

#define USRPH_MAX_SIZE 1024

using namespace std;

class UsrPhrase
{
public:
    void trackUsrInput(const string& text);
    void flush(vector<string>& phrases);
    void clear();

private:
    string m_sentence;
    set<string> m_phrases;
};
#endif
