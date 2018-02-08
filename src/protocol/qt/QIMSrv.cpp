#include <QCursor>

#include "QIMSrv.h"
#include "Application.h"
#include "DBusDaemon.h"

#undef PRINTF
#define PRINTF(fmt, args...)  printf(fmt, ##args)
//#define PRINTF(fmt, args...)

int qic_process_keyevent(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	bool b;
	unsigned int u1, u2, u3, u4 ;
	/* Read the parameters */
	sd_bus_message_read(m, "uuu", &u1, &u2, &u3);
	b = gApp->qim.processKeyEvent(u1, u2, u3);
	return sd_bus_reply_method_return(m, "b", b);
}

int qic_set_cursorlocation(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	int i1, i2, i3, i4;
	sd_bus_message_read(m, "iiii", &i1, &i2, &i3, &i4);
	gApp->qim.getFocus()->setCursorLocation(i1, i2, i3, i4);
	PRINTF("qic_set_cursorlocation, %d, %d, %d, %d\n", i1, i2, i3, i4);
	return sd_bus_reply_method_return(m, NULL);
}

int qic_focusin(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	unsigned int u;
	sd_bus_message_read(m, "u", &u);
	PRINTF("qic_focusin, %u\n", u);
	gApp->qim.processFocusIn(u);
	return sd_bus_reply_method_return(m, NULL);
}

int qic_focusout(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	PRINTF("qic_focusout\n");
	gApp->qim.icManager.focusOut();
	return sd_bus_reply_method_return(m, NULL);   
}

int qic_reset(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	PRINTF("qic_reset\n");
	gApp->qim.getFocus()->reset();
	return sd_bus_reply_method_return(m, NULL);   
}

int qic_enable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	PRINTF("qic_enable\n");
	gApp->qim.getFocus()->enable();
	return sd_bus_reply_method_return(m, NULL);
}

int qic_disable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	PRINTF("qic_disable\n");
	gApp->qim.getFocus()->disable();
	return sd_bus_reply_method_return(m, NULL);
}

int qic_is_enabled(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	PRINTF("qic_is_enabled\n");
	bool b = gApp->qim.getFocus()->isEnabled();
	return sd_bus_reply_method_return(m, "b", b);
}

int qic_set_capabilities(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	PRINTF("qic_set_capabilities\n");
	unsigned int u;
	sd_bus_message_read(m, "u", &u);
	gApp->qim.getFocus()->setCapabilities(u);
	return sd_bus_reply_method_return(m, NULL);
}

int qic_property_activate(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	PRINTF("ping\n");
	const char *s;
	int i;
	sd_bus_message_read(m, "si", &s, &i);
	gApp->qim.getFocus()->propertyActivate(s, i);
	return sd_bus_reply_method_return(m, NULL);
}

int qic_set_surroundingtext(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	PRINTF("qic_set_surroundingtext\n");
	return sd_bus_reply_method_return(m, NULL);
}

int qic_destroy(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	PRINTF("qic_destroy\n");
	gApp->qim.icManager.destroy();
	return sd_bus_reply_method_return(m, NULL);
}

QIMSrv::QIMSrv()
{
	IC *ic = new QIC();  // Add a default IC.
	icManager.add(ic, 0);
}

QIMSrv::~QIMSrv()
{
}

unsigned int QIMSrv::createIC()
{
	IC *ic = new QIC();
	return icManager.add(ic);
}

IC* QIMSrv::getFocus()
{
	return icManager.get();
}

void QIMSrv::onIMOff(void* priv)
{
}

void QIMSrv::onCommit(void* priv, string candidate)
{
	//printf("QIMSrv::onCommit %s\n", candidate.c_str());
	DBusDaemon::getRefrence().qimCommit(candidate);
}

ICRect QIMSrv::onGetRect()
{
    int x = getFocus()->cursorX;
    int y = getFocus()->cursorY + 40;
	return IC::adjRect(x, y, ICWIN_W, ICWIN_H);
}

void QIMSrv::processFocusIn(unsigned int id)
{
	icManager.focusIn(id);

	IC *ic = icManager.get();
	if (ic->id > 0)
		ic->preedit->guiReload(this);
	else {
		IC *ic = new QIC();
		icManager.add(ic, id);
		PRINTF("QIMSrv::processFocusIn, add exists id\n");
	}
}

bool QIMSrv::processKeyEvent(unsigned int keyval, unsigned int keycode, unsigned int state)
{
	PRINTF("QIMSrv::ProcessKeyEvent val: 0x%x,  keycode:0x%x,  state:0x%x \n", keyval, keycode, state);
	if (icManager.get()->preedit->handleKey(keyval, keycode, state, this)  ==  FORWARD_KEY) {
		return false;
	}
	return true; // Eat this key.
}
