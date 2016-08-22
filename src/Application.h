/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include "config.h"
#include "XimSrv.h"
#include "Log.h"
#include "MessageQueue.h"
#include "SysMessager.h"
#include "Signal.h"

class Application
{
public:
    Application();
    ~Application();

    XimSrv  xim;
    MessageQueue *pGuiMsgQ; // Message send to gui
    MessageQueue *pSysMsgQ;
    Signal  sig;
private:
    SysMessager  *m_sysMsgr;
};


extern Application  gApp;

#endif
