#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>

#include "../../aim.h"

#ifndef AIM_RELEASE_MASK
#define AIM_RELEASE_MASK (1u << 30)
#endif

#define AIM_GTK_CONTEXT_ID    IM_ID
#define AIM_GTK_CONTEXT_NAME  IM_NAME

typedef struct _GAimIMContext GAimIMContext;
typedef struct _GAimIMContextClass GAimIMContextClass;

struct _GAimIMContext
{
    GtkIMContext parent;

    GDBusConnection *connection;
    GDBusProxy *bus_proxy;
    GDBusProxy *context_proxy;
    GdkWindow *client_window;
    guint icid;
    gboolean connected;
};

struct _GAimIMContextClass
{
    GtkIMContextClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE(GAimIMContext, gaim_im_context, GTK_TYPE_IM_CONTEXT)

static const GtkIMContextInfo gaim_im_context_info = {
    AIM_GTK_CONTEXT_ID,
    AIM_GTK_CONTEXT_NAME,
    "alphaim",
    "",
    "zh:zh_CN:zh_HK:zh_SG:zh_TW"
};

static const GtkIMContextInfo *gaim_im_context_info_list[] = {
    &gaim_im_context_info
};

static void gaim_im_context_clear_proxy(GAimIMContext *context)
{
    if (context->context_proxy != NULL) {
        g_object_unref(context->context_proxy);
        context->context_proxy = NULL;
    }

    if (context->bus_proxy != NULL) {
        g_object_unref(context->bus_proxy);
        context->bus_proxy = NULL;
    }

    if (context->connection != NULL) {
        g_object_unref(context->connection);
        context->connection = NULL;
    }

    context->icid = 0;
    context->connected = FALSE;
}

static void gaim_im_context_on_signal(GDBusProxy *proxy,
                                      gchar *sender_name,
                                      gchar *signal_name,
                                      GVariant *parameters,
                                      gpointer user_data)
{
    GAimIMContext *context = (GAimIMContext *)user_data;

    if (g_strcmp0(signal_name, "CommitText") == 0) {
        gchar *text = NULL;
        g_variant_get(parameters, "(s)", &text);
        if (text != NULL) {
            g_signal_emit_by_name(context, "commit", text);
            g_free(text);
        }
    }
}

static gboolean gaim_im_context_connect(GAimIMContext *context)
{
    GError *error = NULL;
    GVariant *reply = NULL;

    if (context->connected)
        return TRUE;

    gaim_im_context_clear_proxy(context);

    context->connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
    if (context->connection == NULL) {
        g_clear_error(&error);
        return FALSE;
    }

    context->bus_proxy = g_dbus_proxy_new_sync(context->connection,
                                               G_DBUS_PROXY_FLAGS_NONE,
                                               NULL,
                                               AIM_SRV_NAME,
                                               AIM_SRV_PATH,
                                               AIM_SRV_INTF,
                                               NULL,
                                               &error);
    if (context->bus_proxy == NULL) {
        g_clear_error(&error);
        gaim_im_context_clear_proxy(context);
        g_log(IM_NAME, G_LOG_LEVEL_ERROR,
              "Failed to create D-Bus proxy %s\n",error->message);

        return FALSE;
    }

    reply = g_dbus_proxy_call_sync(context->bus_proxy,
                                   "CreateInputContext",
                                   g_variant_new("(s)", AIM_GTK_IC_NAME),
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);
    if (reply == NULL) {
        g_clear_error(&error);
        gaim_im_context_clear_proxy(context);
        g_log(IM_NAME, G_LOG_LEVEL_ERROR,
              "gtk can't create IC %s\n",error->message);
        return FALSE;
    }

    g_variant_get(reply, "(u)", &context->icid);
    g_variant_unref(reply);

    context->context_proxy = g_dbus_proxy_new_sync(context->connection,
                                                   G_DBUS_PROXY_FLAGS_NONE,
                                                   NULL,
                                                   AIM_SRV_NAME,
                                                   AIM_INPUT_CONTEXT_PATH,
                                                   AIM_INPUT_CONTEXT_INTF,
                                                   NULL,
                                                   &error);
    if (context->context_proxy == NULL) {
        g_clear_error(&error);
        gaim_im_context_clear_proxy(context);
        g_log(IM_NAME, G_LOG_LEVEL_ERROR,
              "gtk can't connect IC %s\n",error->message);
        return FALSE;
    }

    g_signal_connect(context->context_proxy,
                     "g-signal",
                     G_CALLBACK(gaim_im_context_on_signal),
                     context);

    g_dbus_proxy_call(context->context_proxy,
                      "Enable",
                      NULL,
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      NULL,
                      NULL,
                      NULL);

    context->connected = TRUE;
    return TRUE;
}

static gboolean gaim_im_context_filter_keypress(GtkIMContext *im_context,
                                                GdkEventKey *event)
{
    GAimIMContext *context = (GAimIMContext *)im_context;
    GError *error = NULL;
    GVariant *reply;
    guint state;
    gboolean handled = FALSE;

    if (event == NULL || !gaim_im_context_connect(context))
        return FALSE;

    state = event->state;
    if (event->type == GDK_KEY_RELEASE)
        state |= AIM_RELEASE_MASK;

    reply = g_dbus_proxy_call_sync(context->context_proxy,
                                   "ProcessKeyEvent",
                                   g_variant_new("(uuu)",
                                                 (guint)event->keyval,
                                                 (guint)event->hardware_keycode,
                                                 state),
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);
    if (reply == NULL) {
        g_clear_error(&error);
        context->connected = FALSE;
        return FALSE;
    }
    g_log(IM_NAME, G_LOG_LEVEL_DEBUG, "gtk keypress val:%u,code:%u,state:%u\n",
          (guint)event->keyval, (guint)event->hardware_keycode, state);
    g_variant_get(reply, "(b)", &handled);
    g_variant_unref(reply);
    return handled;
}

static void gaim_im_context_focus_in(GtkIMContext *im_context)
{
    GAimIMContext *context = (GAimIMContext *)im_context;

    if (!gaim_im_context_connect(context))
        return;

    g_dbus_proxy_call(context->context_proxy,
                      "FocusIn",
                      g_variant_new("(u)", context->icid),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      NULL,
                      NULL,
                      NULL);
}

static void gaim_im_context_focus_out(GtkIMContext *im_context)
{
    GAimIMContext *context = (GAimIMContext *)im_context;

    if (!context->connected)
        return;

    g_dbus_proxy_call(context->context_proxy,
                      "FocusOut",
                      NULL,
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      NULL,
                      NULL,
                      NULL);
}

static void gaim_im_context_reset(GtkIMContext *im_context)
{
    GAimIMContext *context = (GAimIMContext *)im_context;

    if (!context->connected)
        return;

    g_dbus_proxy_call(context->context_proxy,
                      "Reset",
                      NULL,
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      NULL,
                      NULL,
                      NULL);
}

static void gaim_im_context_set_cursor_location(GtkIMContext *im_context,
                                                GdkRectangle *area)
{
    GAimIMContext *context = (GAimIMContext *)im_context;
    gint x;
    gint y;
    gint origin_x = 0;
    gint origin_y = 0;

    if (area == NULL || !gaim_im_context_connect(context))
        return;

    if (context->client_window != NULL)
        gdk_window_get_origin(context->client_window, &origin_x, &origin_y);

    x = origin_x + area->x;
    y = origin_y + area->y;

    g_dbus_proxy_call(context->context_proxy,
                      "SetCursorLocation",
                      g_variant_new("(iiii)", x, y, area->width, area->height),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      NULL,
                      NULL,
                      NULL);
}

static void gaim_im_context_set_client_window(GtkIMContext *im_context,
                                              GdkWindow *window)
{
    GAimIMContext *context = (GAimIMContext *)im_context;

    if (context->client_window != NULL)
        g_object_unref(context->client_window);

    context->client_window = window;
    if (context->client_window != NULL)
        g_object_ref(context->client_window);
}

static void gaim_im_context_get_preedit_string(GtkIMContext *im_context,
                                               gchar **str,
                                               PangoAttrList **attrs,
                                               gint *cursor_pos)
{
    if (str != NULL)
        *str = g_strdup("");
    if (attrs != NULL)
        *attrs = pango_attr_list_new();
    if (cursor_pos != NULL)
        *cursor_pos = 0;
}

static void gaim_im_context_finalize(GObject *object)
{
    GAimIMContext *context = (GAimIMContext *)object;

    if (context->connected) {
        g_dbus_proxy_call(context->context_proxy,
                          "Destroy",
                          NULL,
                          G_DBUS_CALL_FLAGS_NONE,
                          -1,
                          NULL,
                          NULL,
                          NULL);
    }

    if (context->client_window != NULL) {
        g_object_unref(context->client_window);
        context->client_window = NULL;
    }

    gaim_im_context_clear_proxy(context);
    G_OBJECT_CLASS(gaim_im_context_parent_class)->finalize(object);
}

static void gaim_im_context_class_init(GAimIMContextClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS(klass);

    object_class->finalize = gaim_im_context_finalize;
    im_context_class->filter_keypress = gaim_im_context_filter_keypress;
    im_context_class->focus_in = gaim_im_context_focus_in;
    im_context_class->focus_out = gaim_im_context_focus_out;
    im_context_class->reset = gaim_im_context_reset;
    im_context_class->set_cursor_location = gaim_im_context_set_cursor_location;
    im_context_class->set_client_window = gaim_im_context_set_client_window;
    im_context_class->get_preedit_string = gaim_im_context_get_preedit_string;
}

static void gaim_im_context_class_finalize(GAimIMContextClass *klass)
{
}

static void gaim_im_context_init(GAimIMContext *context)
{
    context->connection = NULL;
    context->bus_proxy = NULL;
    context->context_proxy = NULL;
    context->client_window = NULL;
    context->icid = 0;
    context->connected = FALSE;
}

void im_module_init(GTypeModule *module)
{
    gaim_im_context_register_type(module);
    g_log_set_writer_func(g_log_writer_syslog, NULL, NULL);
}

void im_module_exit(void)
{
}

void im_module_list(const GtkIMContextInfo ***contexts, guint *n_contexts)
{
    *contexts = gaim_im_context_info_list;
    *n_contexts = G_N_ELEMENTS(gaim_im_context_info_list);
}

GtkIMContext *im_module_create(const gchar *context_id)
{
    if (g_strcmp0(context_id, AIM_GTK_CONTEXT_ID) == 0)
        return GTK_IM_CONTEXT(g_object_new(gaim_im_context_get_type(), NULL));

    return NULL;
}
