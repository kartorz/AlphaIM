#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <systemd/sd-bus.h>

#include "Log.h"
#include "MessageQueue.h"
#include "DBusDaemon.h"  // Must after sd-bus.h
#include "Application.h"

static sd_bus *session_bus = NULL;
static sd_bus_slot *session_bus_slot = NULL;

static int dummy_handler(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    return 1;
}

static int im_gui_message(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
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

static int im_create_inputcontext(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    const char *s;
    const char *sender = sd_bus_message_get_sender(m);
    sd_bus_message_read(m, "s", &s);
    if (sender && (gApp->dim.getClient(s) != DBUS_CLIENT_UNKNOWN)) {
        unsigned int u = gApp->dim.createIC(s, sender);
        return sd_bus_reply_method_return(m, "u", u);
    }
    logger.e("im_create_inputcontext: sender is NULL\n");
    return sd_bus_reply_method_return(m, NULL);
}

static int im_exit(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("exit\n");
    return 0;
}

/* A custom command line: eg: got im activate */
static int im_ping(sd_bus_message *m, void *userdata, sd_bus_error *ret_error)
{
    PRINTF("ping\n");
    return sd_bus_reply_method_return(m, "v", "s", "aa");
}

static  sd_bus_vtable im_vtable[] = {
    SD_BUS_VTABLE_START(0),

    SD_BUS_METHOD("GuiMessage", "i", "i", im_gui_message, 0),
    SD_BUS_METHOD("CreateInputContext", "s", "u", im_create_inputcontext, 0),
    SD_BUS_METHOD("Exit", "b", "", im_exit, 0),
    SD_BUS_METHOD("Ping", "v", "v", im_ping, 0),

    SD_BUS_VTABLE_END
};

static  sd_bus_vtable ic_vtable[] = {
    SD_BUS_VTABLE_START(0),

    SD_BUS_METHOD("ProcessKeyEvent", "uuuu", "i", ic_process_keyevent, 0),
    SD_BUS_METHOD("SetCursorLocation", "uiiii", "i", ic_set_cursorlocation, 0),
    SD_BUS_METHOD("FocusIn", "u", "i", ic_focusin, 0),
    SD_BUS_METHOD("FocusOut", "u", "i", ic_focusout, 0),
    SD_BUS_METHOD("Reset", "u", "i", ic_reset, 0),
    SD_BUS_METHOD("Enable", "u", "i", ic_enable, 0),
    SD_BUS_METHOD("Disable", "u", "i", ic_disable, 0),
    SD_BUS_METHOD("IsEnabled", "u", "b", ic_is_enabled, 0),
    SD_BUS_METHOD("SetCapabilities", "uu", "i", ic_set_capabilities, 0),
    SD_BUS_METHOD("PropertyActivate", "usi", "i", ic_property_activate, 0),
    SD_BUS_METHOD("SetSurroundingText", "uvuu", "i", ic_set_surroundingtext, 0),
    SD_BUS_METHOD("Destroy", "u", "i", ic_destroy, 0),

    SD_BUS_VTABLE_END
};

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
    if (r < 0) {
        logger.e("dbus_daemon_register, get user bus: error code: %s\n", strerror(-r));
        return -1;
    }

    /* Install the object */
    r = sd_bus_add_object_vtable(session_bus, &session_bus_slot, AIM_SRV_PATH, AIM_SRV_INTF, im_vtable, NULL);
    if (r < 0) {
        logger.e("dbus_daemon_register, add im vtable: error code: %s\n", strerror(-r));
        sd_bus_unref(session_bus);
        return -2;
    }

    r = sd_bus_add_object_vtable(session_bus, &session_bus_slot, AIM_INPUT_CONTEXT_PATH, AIM_INPUT_CONTEXT_INTF, ic_vtable, NULL);
    if (r < 0) {
        logger.e("dbus_daemon_register, add dbic vtable: error code: %s\n", strerror(-r));
        goto err;
    }

    /* Take a well-known service name so that clients can find us */
    r = sd_bus_request_name(session_bus, AIM_SRV_NAME, 0);
    if (r < 0) {
        logger.e("dbus_daemon_register, request name: error code: %s\n", strerror(-r));
        goto err;
    }

    return 0;
err:
    sd_bus_slot_unref(session_bus_slot);
    sd_bus_unref(session_bus);
    session_bus_slot = NULL;
    return -2;
}

void DBusDaemon::finish()
{
    if (session_bus_slot != NULL) {
        sd_bus_slot_unref(session_bus_slot);
        session_bus_slot = NULL;
    }

    if (session_bus != NULL) {
        sd_bus_unref(session_bus);
        session_bus = NULL;
    }
}

void DBusDaemon::listen()
{
    int r = sd_bus_process(session_bus, NULL);
    if (r < 0) {
        logger.e("D-Bus daemon finished, error: %d\n", r);
        finish();
        return;
    }
    if (r > 0) /* we processed a request, try to process another one, right-away */
        return;

    /* Wait for the next request to process */
    r = sd_bus_wait(session_bus, (uint64_t) -1);
    if (r < 0) {
        logger.e("D-Bus daemon finished, error: %d\n", r);
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

int DBusDaemon::dbusIMCommit(std::string& candidate)
{
    int r;
    sd_bus_message *m = NULL;
    r = sd_bus_message_new_signal(session_bus, &m, AIM_INPUT_CONTEXT_PATH, AIM_INPUT_CONTEXT_INTF, "CommitText");
    if (!m) {
        logger.e("DBusDaemon::dbusIMCommit  new signal error, err: %s\n", strerror(-r));
        return -1;
    }
    r = sd_bus_message_append(m, "s",  candidate.c_str());
    r = sd_bus_send(session_bus, m, NULL);
    if (r < 0) {
        logger.e("DBusDaemon::dbusIMCommit send error, err: %s\n",strerror(-r));
        return -3;
    }

    PRINTF("DBusDaemon::dbusIMCommit\n");
    return 0;
}

int DBusDaemon::notify(Message& msg)
{
    int r;
    sd_bus_message *m = NULL;
    r = sd_bus_message_new_signal(session_bus, &m, AIM_NOTIFY_PATH, AIM_NOTIFY_INTF, AIM_NOTIFY_MESSAGE);
    if (!m) {
        logger.e("DBusDaemon::notify  new signal error, err: %s\n", strerror(-r));
        return -1;
    }

    if (msg.id == MSG_IM_INPUT) {
        r = sd_bus_message_append(m, "iiiiiss",  msg.id, msg.iArg1, msg.iArg2, (int)msg.fArg1, (int)msg.fArg2, msg.strArg1.c_str(), msg.strArg2.c_str());
    } else {
        r = sd_bus_message_append(m, "i", msg.id);
    }
    if (r < 0) {
        logger.e("DBusDaemon::notify append data error, err: %s\n", strerror(-r));
        return -2;
    }

    r = sd_bus_send(session_bus, m, NULL);
    if (r < 0) {
        logger.e("DBusDaemon::notify send error, err: %s\n", strerror(-r));
        return -3;
    }
    //PRINTF("DBusDaemon::notify id: %d\n", msg.id);
    return 0;
}

int DBusDaemon::signal(int id)
{
    int r;
    sd_bus_message *m = NULL;
    r = sd_bus_message_new_signal(session_bus, &m, AIM_NOTIFY_PATH, AIM_NOTIFY_INTF, AIM_NOTIFY_MESSAGE);
    if (!m) {
        logger.e("DBusDaemon::notify  new signal error, err: %s\n", strerror(-r));
        return -1;
    }

    r = sd_bus_message_append(m, "i", id);
    if (r < 0) {
        logger.e("DBusDaemon::notify append data error, err: %s\n", strerror(-r));
        return -2;
    }

    r = sd_bus_send(session_bus, m, NULL);
    if (r < 0) {
        logger.e("DBusDaemon::notify send error, err: %s\n", strerror(-r));
        return -3;
    }
    return 0;
}

void DBusDaemon::doWork()
{
    listen();
}

void DBusDaemon::onExit()
{
    finish();
    logger.i("DBusDaemon::onExit\n");
}

void DBusDaemon::stop()
{
    abort();

    callGuiMessage(MSG_QUIT);
}
