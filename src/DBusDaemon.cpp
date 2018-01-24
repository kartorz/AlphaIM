#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <systemd/sd-bus.h>

#include "Log.h"
#include "MessageQueue.h"
#include "DBusDaemon.h"  // Must after sd-bus.h
#include "Application.h"

#undef PRINTF
//#define PRINTF(fmt, args...)  printf(fmt, ##args)
#define PRINTF(fmt, args...)

static sd_bus *session_bus = NULL;
static sd_bus_slot *session_bus_slot = NULL;

static int dummy_handler(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	return 1;
}

static int gui_message(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
	int id, r;
	/* Read the parameters */
	r = sd_bus_message_read(m, "i", &id);
	if (r < 0) {
		printf("gui message read arg, error code: %s\n", strerror(-r));
		return r;
	}

	PRINTF("gui_message read id %d\n", id);
	if (id >= MSG_UI_LAN  && id <= MSG_UI_PUN)
		gApp->xim.handleUIMessage(id);

	/* Reply with the response */
	return sd_bus_reply_method_return(m, "i", 1);
}

inline sd_bus_vtable setup_method(const char *member, const char *signature, const char *result, sd_bus_message_handler_t handler)
{
	sd_bus_vtable t;
	t.type = _SD_BUS_VTABLE_METHOD;
	t.flags = SD_BUS_VTABLE_UNPRIVILEGED;
	t.x.method.member = member;
	t.x.method.signature = signature;
	t.x.method.result = result;
	t.x.method.handler = handler;
	t.x.method.offset = 0;
	return t;
}
static  sd_bus_vtable im_vtable[3];
static void setup_vtable()
{
	int i = 0;
	sd_bus_vtable s;
	s.type = _SD_BUS_VTABLE_START;
    s.flags = 0;
	s.x.start.element_size = sizeof(sd_bus_vtable);
	im_vtable[i++] = s;

	im_vtable[i++] = setup_method("GuiMessage", "i", "i", gui_message);

	sd_bus_vtable e;
	e.type = _SD_BUS_VTABLE_END;
	im_vtable[i] = e;
}

DBusDaemon& DBusDaemon::getRefrence()
{
    static DBusDaemon dbusDaemon;
    return dbusDaemon;
}

DBusDaemon::DBusDaemon(): Thread(0)
{
}

// Debug:
//    busctl  --user  introspect   org.freedesktop.AlphaIM  /org/freedesktop/AlphaIM
//    busctl  --user  call   org.freedesktop.AlphaIM  /org/freedesktop/AlphaIM org.freedesktop.AlphaIM  GuiMessage i  10

int DBusDaemon::setup()
{
	int r;
	/* Connect to the user bus this time */
	r  = sd_bus_default_user(&session_bus);
	//r  = sd_bus_open_user(&session_bus);
	if (r < 0) {
		log.e("dbus_daemon_register, get user bus: error code: %s\n", strerror(-r));
		return -1;
	}

	/* Install the object */
	setup_vtable();
	r = sd_bus_add_object_vtable(session_bus, &session_bus_slot, AIM_SRV_PATH, AIM_SRV_INTF, im_vtable, NULL);
	if (r < 0) {
		log.e("dbus_daemon_register, add vtable: error code: %s\n", strerror(-r));
		sd_bus_unref(session_bus);
		return -2;
	}

	/* Take a well-known service name so that clients can find us */
	r = sd_bus_request_name(session_bus, AIM_SRV_NAME, 0);
	if (r < 0) {
		log.e("dbus_daemon_register, request name: error code: %s\n", strerror(-r));
		sd_bus_unref(session_bus);
		sd_bus_slot_unref(session_bus_slot);
		return -3;
	}

	return 0;
}

void DBusDaemon::finish()
{
	sd_bus_slot_unref(session_bus_slot);
	sd_bus_unref(session_bus);
}

void DBusDaemon::listen()
{
	int r = sd_bus_process(session_bus, NULL);
	if (r < 0) {
		log.e("D-Bus daemon finished, error: %d\n", r);
		finish();
		return;
	}
	if (r > 0) /* we processed a request, try to process another one, right-away */
		return;

	/* Wait for the next request to process */
	r = sd_bus_wait(session_bus, (uint64_t) -1);
	if (r < 0) {
		log.e("D-Bus daemon finished, error: %d\n", r);
		finish();
		abort();
		return;
	}
}

int DBusDaemon::callGuiMessage(int id)
{
	int r = sd_bus_call_method_async(session_bus, NULL, AIM_SRV_NAME, AIM_SRV_PATH, AIM_SRV_INTF,
									 "GuiMessage", dummy_handler, NULL, "i", id);
	if (r < 0)
		printf("callGuiMessage, error code: %s\n", strerror(-r));

	return r;
}

int DBusDaemon::notify(Message& msg)
{
	int r;
	sd_bus_message *m = NULL;
	r = sd_bus_message_new_signal(session_bus, &m, AIM_NOTIFY_PATH, AIM_NOTIFY_INTF, AIM_NOTIFY_MESSAGE);
	if (!m) {
		log.e("DBusDaemon::notify  new signal error, err: %d\n", r);
		return -1;
	}

	if (msg.id == MSG_IM_INPUT) {
		r = sd_bus_message_append(m, "iiiiiss",  msg.id, msg.iArg1, msg.iArg2, (int)msg.fArg1, (int)msg.fArg2, msg.strArg1.c_str(), msg.strArg2.c_str());
	} else {
		r = sd_bus_message_append(m, "i", msg.id);
	}
	if (r < 0) {
		log.e("DBusDaemon::notify append data error, err: %d\n", r);
		return -2;
	}

	r = sd_bus_send(session_bus, m, NULL);
	if (r < 0) {
		log.e("DBusDaemon::notify send error, err: %d\n", r);
		return -3;
	}

	PRINTF("DBusDaemon::notify id: %d\n", msg.id);
}

int DBusDaemon::signal(int id)
{
	int r;
	sd_bus_message *m = NULL;
	r = sd_bus_message_new_signal(session_bus, &m, AIM_NOTIFY_PATH, AIM_NOTIFY_INTF, AIM_NOTIFY_MESSAGE);
	if (!m) {
		log.e("DBusDaemon::notify  new signal error, err: %d\n", r);
		return -1;
	}

	r = sd_bus_message_append(m, "i", id);
	if (r < 0) {
		log.e("DBusDaemon::notify append data error, err: %d\n", r);
		return -2;
	}

	r = sd_bus_send(session_bus, m, NULL);
	if (r < 0) {
		log.e("DBusDaemon::notify send error, err: %d\n", r);
		return -3;
	}
}

void DBusDaemon::doWork()
{
	listen();
}

void DBusDaemon::onExit()
{
	finish();
	log.i("DBusDaemon::onExit\n");
}

void DBusDaemon::stop()
{
	abort();

	callGuiMessage(MSG_QUIT);
}
