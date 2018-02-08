/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _IC_H_
#define _IC_H_

#include <map>

#include "IMPreedit.h"

#define ICWIN_W  768
#define ICWIN_H  64

class IC {
public:
    IC();
	virtual ~IC();

	virtual void enable();
	virtual void disable();
    virtual bool isEnabled();
	virtual void setCursorLocation(int x, int y, int w,  int h);
	virtual void reset();
	virtual void setCapabilities(unsigned int cpas);
	virtual void propertyActivate(const char *name, int state);
	virtual void setSurroundingText();
	virtual void close();
	static ICRect adjRect(int x, int y, int w, int h);

	unsigned int id;
	int   cursorX;
	int   cursorY;
    IMPreedit *preedit;

	static int dpyW;
    static int dpyH;
};

class ICManager {
public:
    ICManager();
	~ICManager();

	unsigned int add(IC *ic);
	void add(IC *ic, int id);
	void destroy(unsigned int id = 0);
	void focusIn(unsigned int id = 0);
	IC* get(unsigned int id = 0);
	void focusOut();

private:
	std::map<int,  IC*> m_ics;
	int  m_icid;
	int  m_icFocus;
	//MutexCriticalSection m_cs;
};

#endif
