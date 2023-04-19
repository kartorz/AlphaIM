/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _DBUS_DAEMON_H_
#define _DBUS_DAEMON_H_

#include "aim.h"

class DBusDaemon :public Thread
{
public:
    static DBusDaemon& getRefrence();

    DBusDaemon();
    virtual ~DBusDaemon() {}

    int setup();
    void listen();
    int notify(Message& msg);
    int qimCommit(std::string& candidate);
    int signal(int id);
    void finish();
    int  callGuiMessage(int id);

    virtual void stop();

protected:
    virtual void doWork();
    virtual void onExit();
};

#endif
