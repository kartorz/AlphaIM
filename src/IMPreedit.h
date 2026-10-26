#ifndef _IMPREEDIT_H_
#define _IMPREEDIT_H_

#include <string>
#include <deque>
#include "MutexLock.h"

#include "aim.h"
#include "iIM.h"
#include "MessageQueue.h"
#include "CharUtil.h"
#include "UsrPhrase.h"

/*
 * Situation:
 *     vv: a valid input.
 *     hansx: chinese character.
 *     hans1: has be selected by pressing digital key.
 *     cc   : is going to be slelected by user pressing digital key.
 *     dd   : Another valid py.
 *
 *  input sequences         vv - bb    - vv - cc    - dd
 *                          vv - hans1 - vv - hans2 - hans3
 *
 * =================================================================
 *  ci            vvhans1
 *  input         vvccdd
 *  candidate     vvhans2(0)hans3(0)
 *
 *  items         hans2
 *  ci_items      hans1
 *
 * ================================================================
 * IM UI:
 * input        ci + input
 * candidate    ci + candidate.
 * items        items
 */

using namespace std;


typedef struct {
    int x,y,w,h;
} ICRect;

typedef struct {
    int    keysym;
    int    modifier;
    int    modifier_mask;
} TriggerKey;

class IC;

enum PREEDIT_KEY {
    NONE_KEY,
    FORWARD_KEY,
    TRIGGER_ON_KEY,
    TRIGGER_OFF_KEY,
    SWITCH_CE_KEY,
    SWITCH_CEP_KEY,
    CONVERT_KEY,
    COMMIT_KEY,
    PAGEUP_KEY,
    PAGEDOWN_KEY,
    CANCEL_KEY,
    PREEDIT_KEY
};

class IMPreeditCallback {
public:
    IMPreeditCallback(void* priv=NULL): opaque(priv) {};
    virtual void onIMOn(void* priv) {};
    virtual void onIMOff(void* priv) {};
    virtual void onCommit(void* priv, string candidate) = 0;
    void *opaque;
};

class IMPreedit {
public:
    IMPreedit();
    virtual ~IMPreedit();
    inline void addRef() {++refs;};
    inline bool releaseRef() {return refs > 0 && --refs == 0;};
    virtual void handleMessage(int msg, int arg = 0);
    virtual int doHandleKey(IC* ic, u32 key, u32 modifier, IMPreeditCallback *callback) = 0;
    virtual bool imWants(u32 key, u32 modifier) = 0;

    void guiReload();
    void guiShowCandidate(IC *ic);
    bool isActCN() const { return m_bTrigger && m_bCN; }
    void clear();
    void close();
    void reset();

    u32 refs;

protected:
    void doTriggerOn();
    void doTriggerOff();
    bool doSwitchCE();
    bool doSwitchCEPun();
    void doPageup();
    void doPagedown();
    bool doInput(char key);
    void doClose();
    void doCommit(int i, IMPreeditCallback *callback);
    void flushUsrInput();

    void guiAction(int id, int flags);

    u32  mapCNPun(char key) const;
    string  mapCNPunToU8Str(char key);
    string  lookup(string input);

    bool m_bStart;
    string m_input;
    string m_ci;
    string m_candidate;
    int m_uiStringMax;

    bool   m_bTrigger;
    bool   m_bCN;
    bool   m_bCNPun;  //Chinese punctuation
    bool   m_bPreQuo;
    bool   m_bPreSingleQuo;

private:
    void add(char key);
    void del();
    void page(int pg);
    bool commit(int i);

    MutexCriticalSection m_cs;
    UsrPhrase m_usrPhrase;

    int m_curPage; // Start from 1.
    deque<int> m_pageWin;
    deque<IMItem> m_items;
    deque<IMItem> m_ciItems;
    deque<IMItem> m_uiItems;
};

#endif
