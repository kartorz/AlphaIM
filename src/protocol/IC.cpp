#include "IC.h"

#undef PRINTF
//#define PRINTF(fmt, args...)  printf(fmt, ##args)
#define PRINTF(fmt, args...)

int IC::dpyW = 0;
int IC::dpyH = 0;

IC::IC(): id(0),cursorX(0),cursorY(0)
{
}

IC::~IC()
{
	delete preedit;
}

void IC::close()
{
	preedit->close();
}

void IC::setCursorLocation(int x, int y, int w, int h)
{
	//printf("IC::setCursorLocation (%d, %d, %d, %d)\n", x, y, w, h);
	cursorX = x;
	cursorY = y;
}

void IC::reset()
{
	PRINTF("reset %d\n", id);
	preedit->reset();
}

void IC::enable()
{
}

void IC::disable()
{
}

bool IC::isEnabled()
{
	return true;
}

void IC::setCapabilities(unsigned int cpas)
{
}

void IC::propertyActivate(const char *name, int state)
{
}

void IC::setSurroundingText()
{
}

ICRect IC::adjRect(int x, int y, int w, int h)
{
	ICRect ret;

	if ((x == 0 || y == 0) && dpyW > 0) {
		// Client don't set cursor.
		x = (dpyW - ICWIN_W)/2;
		y = dpyH - ICWIN_H - 60;
	}

	if (x + w > dpyW - 20) {
		x = dpyW - ICWIN_W - 20;
	}

	if (y + h > dpyH - 10) {
		y = y - 2*h;
	}
	//PRINTF("getICWin (%d, %d, %d, %d)\n",  x, y, w, h);
	ret.x = x;
	ret.y = y;
	ret.w = w;
	ret.h = h;
	return ret;
}

ICManager::ICManager():  m_icid(0), m_icFocus(0)
{
}

ICManager::~ICManager()
{
    std::map<int, IC*>::iterator iter;
    for (iter = m_ics.begin(); iter != m_ics.end(); iter++)
        delete(iter->second);
}

void ICManager::focusIn(unsigned int id)
{	
	m_icFocus = id;
	PRINTF("focusIn %d\n", m_icFocus);
}

void ICManager::focusOut()
{
	PRINTF("focusOut %d\n", m_icFocus);
	get()->preedit->reset();
	m_icFocus = 0;
}

unsigned int ICManager::add(IC *ic)
{
    m_ics[++m_icid] = ic;
	ic->id = m_icid;
	m_icFocus = m_icid;	
    return ic->id;
}


// 'AlphaIM' crashes, pos is largger.
void ICManager::add(IC *ic, int id)
{
	m_ics[id] = ic;
	ic->id = id;
	m_icFocus = id;
	if (m_icid < id)
		m_icid = id;
}

void ICManager::destroy(unsigned int id)
{
	int icid = id == 0 ? m_icFocus : id;
	if (icid > 0) {
		std::map<int, IC*>::iterator iter = m_ics.find(icid);
		if(iter != m_ics.end()) {
			delete(iter->second);
			m_ics.erase(iter);
			//printf("destroy id:%d\n", icid);
		}
	}
}

IC* ICManager::get(unsigned int id)
{
	int icid = id == 0 ? m_icFocus : id;
	//printf("ICManager::get id: %d\n", icid);
    std::map<int, IC*>::iterator iter = m_ics.find(icid);
    if(iter != m_ics.end()) {
        return iter->second;
    }
	//printf("ICManager::get return default IC\n");
    return m_ics[0];
}
