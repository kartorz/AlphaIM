/** 
 *	@Copyright (c) 2016 joni <joni.kartorz.lee@gmail.com>
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */

#ifndef _QIMSRV_H_
#define _QIMSRV_H_

#include <systemd/sd-bus.h>
#include <string>

#include "IC.h"
#include "QtIMPreedit.h"

class QIC : public IC {
public:
    QIC() {
		preedit = new QtIMPreedit();
	}
	virtual ~QIC(){}
};

class QIMSrv : public IMPreeditCallback {
public:
    QIMSrv();
	~QIMSrv();
	void processFocusIn(unsigned int id);
	bool processKeyEvent(unsigned int keyval, unsigned int keycode, unsigned int state);
	unsigned int createIC();
	virtual void onIMOff(void* priv);
    virtual void onCommit(void* priv, string candidate);
    virtual ICRect onGetRect();
	IC* getFocus();
	ICManager icManager;
};

extern int qic_process_keyevent(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_set_cursorlocation(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_focusin(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_focusout(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_reset(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_enable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_disable(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_is_enabled(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_set_capabilities(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_property_activate(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_set_surroundingtext(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);
extern int qic_destroy(sd_bus_message *m, void *userdata, sd_bus_error *ret_error);

#endif
