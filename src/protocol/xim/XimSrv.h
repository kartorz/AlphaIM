/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _XIMSRV_H_
#define _XIMSRV_H_

#include <X11/Xlib.h>
#include "Ximd/IMdkit.h"
#include "Ximd/Xi18n.h"
#include "XIC.h"
#include "iIM.h"
#include "TaskManager.h"

using namespace std;

class XimSrv : public IMPreeditCallback
{
friend class ModifierKeyEventTask;
public:
    XimSrv();
    ~XimSrv();

    bool open(Display *dpy);
    void close();
    int handleIMOpen(XIMS ims, IMProtocol *calldata);
    int handleIMCreateIC(XIMS ims, IMProtocol *calldata);
    int handleIMSetICValues(XIMS ims, IMProtocol *calldata);
    int handleGetICValues(XIMS ims, IMProtocol *calldata);
    int handleForwardEvent(XIMS ims, IMProtocol *calldata);
    int handleSetICFocusEvent(XIMS ims, IMProtocol *calldata);
    int handleUnsetICFocusEvent(XIMS ims, IMProtocol *calldata);
    int handleResetICEvent(XIMS ims, IMProtocol *calldata);
    int handleTriggerNotify(XIMS ims, IMProtocol *calldata);
    int handlePreeditStartReply(XIMS ims, IMProtocol *calldata);
    int handlePreeditCaretReply(XIMS ims, IMProtocol *calldata);

    int doModifierKeyEvent(XIMS ims, IMProtocol *calldata);

    void setIM(iIM *im, bool en);

    virtual void onIMOff(void* priv);
    virtual void onCommit(void* priv, string candiate);
    virtual ICRect onGetRect(void* priv);

private:
    void closeIC(IC* ic);
    void commit(XIMS ims, IMForwardEventStruct* calldata, string candidate);
    XRectangle getICWinRect();

    Xicm m_icMgr;
    iIM  *m_im;

    XIMS m_ims;
    Window   m_imwin;
    Display* m_dpy;
    int      m_dpyW;
    int      m_dpyH;

    bool m_bDynamicEvent;
    int m_preModKey;
    //ModifierKeyEventTask*  m_modifierKeyTask[2];
};

#endif
