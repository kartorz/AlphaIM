/*
 * Test: ./gedit, evolution,  PRINTF output.
 * -------------------
 *   g_signal_emit_by_name(context, "commit", "a");
 *
 * Click at a input
 *   -gaim_ic_focus_in, [41]
 *   -gaim_ic_reset, [41]
 *
 */

#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#if GTK_CHECK_VERSION(4, 0, 0)
#include <gio/giomodule.h>
#include <gdk/x11/gdkx.h>
#include <X11/Xlib.h>
#endif

#include "../../aim.h"

#define AIM_GTK_CONTEXT_ID    IM_ID
#define AIM_GTK_CONTEXT_NAME  IM_NAME

/* Commit text to focus IC, one IC per input widget.
 * -------------------------------------------------
 * g_im_active:
 *   0: don't forward normal input events.
 *   1: forward all events.
 */
static guint g_ic_focus = 0;
static gint  g_im_active = 0;

typedef struct _GAimIMContext GAimIMContext;
typedef struct _GAimIMContextClass GAimIMContextClass;

struct _GAimIMContext
{
    GtkIMContext parent;

    GDBusConnection *connection;
    GDBusProxy *bus_proxy;
    GDBusProxy *context_proxy;
    GtkIMContext *fallback;
#if GTK_CHECK_VERSION(4, 0, 0)
    GtkWidget *client_widget;
#else
    GdkWindow *client_window;
#endif
    guint icid;
    gboolean connected;
};

struct _GAimIMContextClass
{
    GtkIMContextClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE(GAimIMContext, gaim_im_context, GTK_TYPE_IM_CONTEXT)

#if !GTK_CHECK_VERSION(4, 0, 0)
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
#endif

static gboolean is_key_release(
#if GTK_CHECK_VERSION(4, 0, 0)
    GdkEvent *event)
#else
    GdkEventKey *event)
#endif
{
#if GTK_CHECK_VERSION(4, 0, 0)
    return gdk_event_get_event_type(event) == GDK_KEY_RELEASE;
#else
    return event->type == GDK_KEY_RELEASE;
#endif
}
// Sync with OnOffKeys within X11Preedit
static gboolean is_trigger_key(guint keyval, guint state)
{
    return keyval == GDK_KEY_space &&
           (((state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK) ||
            ((state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK) ||
            ((state & GDK_SUPER_MASK) == GDK_SUPER_MASK));
}

static void ic_on_signal(GDBusProxy *proxy,
                                      gchar *sender_name,
                                      gchar *signal_name,
                                      GVariant *parameters,
                                      gpointer user_data)
{
    GAimIMContext *context = (GAimIMContext *)user_data;

    if ((context->icid == g_ic_focus) &&
        (g_strcmp0(signal_name, "CommitText") == 0)) {
        gchar *text = NULL;
        g_variant_get(parameters, "(s)", &text);
        if (text != NULL) {
            g_signal_emit_by_name(context, "commit", text);
            g_free(text);
        }
        PRINTF("ic_on_signal, id:[%d]\n", context->icid);
    }
}

static void ic_disconnect(GAimIMContext *context)
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

    PRINTF("ic_disconnect id:[%d]\n", context->icid);
    context->icid = 0;
    context->connected = FALSE;
}

static gboolean ic_connect(GAimIMContext *context)
{
    GError *error = NULL;
    GVariant *reply = NULL;

    if (context->connected)
        return TRUE;

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
        g_log(IM_NAME, G_LOG_LEVEL_ERROR,
              "Failed to create D-Bus proxy %s\n",error->message);
        g_clear_error(&error);
        ic_disconnect(context);
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
        g_log(IM_NAME, G_LOG_LEVEL_ERROR,
              "gtk can't create IC %s\n",error->message);
        g_clear_error(&error);
        ic_disconnect(context);
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
        g_log(IM_NAME, G_LOG_LEVEL_ERROR,
              "gtk can't connect IC %s\n",error->message);
        g_clear_error(&error);
        ic_disconnect(context);
        return FALSE;
    }

    g_signal_connect(context->context_proxy,
                     "g-signal",
                     G_CALLBACK(ic_on_signal),
                     context);

    context->connected = TRUE;
    PRINTF("ic_connect, id:[%d]\n", context->icid);
    return TRUE;
}

static void gaim_ic_on_fallback_commit(GtkIMContext *fallback,
                                       const gchar *text,
                                       gpointer user_data)
{
    GAimIMContext *context = (GAimIMContext *)user_data;

    g_signal_emit_by_name(context, "commit", text);
}

static gboolean gaim_ic_filter_fallback(GAimIMContext *context,
#if GTK_CHECK_VERSION(4, 0, 0)
                                        GdkEvent *event)
#else
                                        GdkEventKey *event)
#endif
{
    if (context->fallback == NULL)
        return FALSE;

    return gtk_im_context_filter_keypress(context->fallback, event);
}

static gboolean gaim_ic_filter_keypress(GtkIMContext *im_context,
#if GTK_CHECK_VERSION(4, 0, 0)
                                                GdkEvent *event)
#else
                                                GdkEventKey *event)
#endif
{
    GAimIMContext *context = (GAimIMContext *)im_context;
    GError *error = NULL;
    GVariant *reply;
    guint state;
    guint keyval;
    gboolean keyrls;
    gboolean trigger_key = FALSE;
    gint handled = 0;

    if (event == NULL)
        return FALSE;

#if GTK_CHECK_VERSION(4, 0, 0)
    state = gdk_event_get_modifier_state(event);
    keyval = gdk_key_event_get_keyval(event);
    keyrls = is_key_release(event);
#else
    state = event->state;
    keyval = event->keyval;
    keyrls = is_key_release(event);
#endif
    if (keyrls) {
        state |= AIM_RELEASE_MASK;
        trigger_key = is_trigger_key(keyval, state);
    }

    if ((!trigger_key && !g_im_active) || !context->connected)
        return gaim_ic_filter_fallback(context, event);

    reply = g_dbus_proxy_call_sync(context->context_proxy,
                                   "ProcessKeyEvent",
                                   g_variant_new("(uuuu)",
                                       context->icid,
#if GTK_CHECK_VERSION(4, 0, 0)
                                       keyval,
                                       gdk_key_event_get_keycode(event),
#else
                                       keyval,
                                       (guint)event->hardware_keycode,
#endif
                                       state),
                                   G_DBUS_CALL_FLAGS_NONE,
                                   -1,
                                   NULL,
                                   &error);
    if (reply == NULL) {
        g_log(IM_NAME, G_LOG_LEVEL_ERROR,
              "keypress reconnect, err:%s\n",
              error != NULL ? error->message : "unknown error");
        g_clear_error(&error);
        return gaim_ic_filter_fallback(context, event);
    }

    g_variant_get(reply, "(i)", &handled);
    g_variant_unref(reply);

    //PRINTF("gaim_ic_filter_keypress: ic:[%d], ac:[%d],return:%d\n", context->icid, g_im_active, handled);

    if (!handled)
        return gaim_ic_filter_fallback(context, event);

    g_im_active = handled - 1;

    return TRUE;
}


static void gaim_ic_focus_in(GtkIMContext *im_context)
{
    GAimIMContext *context = (GAimIMContext *)im_context;

    gtk_im_context_focus_in(context->fallback);
    g_ic_focus = context->icid;
    if (!context->connected)
        return;

    GError *error = NULL;
    GVariant *reply = g_dbus_proxy_call_sync(
        context->context_proxy,
        "FocusIn",
        g_variant_new("(u)", context->icid),
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        &error);

    if (!reply) { 
        g_log(IM_NAME, G_LOG_LEVEL_ERROR,
              "Failed to call FocusIn %s\n",error->message);
        g_clear_error(&error);
        return;
    }
    g_variant_get(reply, "(i)", &g_im_active);
    g_variant_unref(reply);
    PRINTF("[%d]:gaim_ic_focus_in, active:%d\n", context->icid, g_im_active);
}

static void gaim_ic_focus_out(GtkIMContext *im_context)
{
    GAimIMContext *context = (GAimIMContext *)im_context;

    gtk_im_context_focus_out(context->fallback);

    g_ic_focus = 0;
    if (!context->connected || !g_im_active)
        return;

    g_dbus_proxy_call(context->context_proxy,
                      "FocusOut",
                      g_variant_new("(u)", context->icid),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      NULL,
                      NULL,
                      NULL);
    PRINTF("gaim_ic_focus_out, [%d]\n", context->icid);
}

static void gaim_ic_reset(GtkIMContext *im_context)
{
    GAimIMContext *context = (GAimIMContext *)im_context;

    gtk_im_context_reset(context->fallback);

    if (!context->connected || !g_im_active)
        return;

    g_dbus_proxy_call(context->context_proxy,
                      "Reset",
                      g_variant_new("(u)", context->icid),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      NULL,
                      NULL,
                      NULL);
    PRINTF("gaim_ic_reset, [%d]\n", context->icid);
    //g_signal_emit_by_name(im_context, "preedit-changed");
}

static void gaim_ic_set_cursor_location(GtkIMContext *im_context,
                                                GdkRectangle *area)
{
    GAimIMContext *context = (GAimIMContext *)im_context;
    gint x;
    gint y;
    gint origin_x = 0;
    gint origin_y = 0;

    gtk_im_context_set_cursor_location(context->fallback, area);

    if (area == NULL || !context->connected || !g_im_active)
        return;

#if !GTK_CHECK_VERSION(4, 0, 0)
    if (context->client_window != NULL)
        gdk_window_get_origin(context->client_window, &origin_x, &origin_y);
#elif defined(GDK_WINDOWING_X11)
    if (context->client_widget != NULL) {
        GtkNative *native = gtk_widget_get_native(context->client_widget);
        if (native != NULL) {
            GdkSurface *surface = gtk_native_get_surface(native);
            if (surface != NULL) {
                GdkDisplay *display = gdk_surface_get_display(surface);
                if (GDK_IS_X11_DISPLAY(display)) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
                    Display *xdisplay = gdk_x11_display_get_xdisplay(display);
                    Window xwindow = gdk_x11_surface_get_xid(surface);
                    Window root = gdk_x11_display_get_xrootwindow(display);
                    Window child;
#pragma GCC diagnostic pop
                    XTranslateCoordinates(xdisplay,
                                           xwindow,
                                           root,
                                           0, 0,
                                           &origin_x,
                                           &origin_y,
                                           &child);
                    PRINTF("SetCursor:fallback x11, (%d,%d)\n", origin_x, origin_y);
                }
            }
        }
    }
#endif

    x = origin_x + area->x;
    y = origin_y + area->y;

    g_dbus_proxy_call(context->context_proxy,
                      "SetCursorLocation",
                      g_variant_new("(uiiii)", context->icid, x, y, area->width, area->height),
                      G_DBUS_CALL_FLAGS_NONE,
                      -1,
                      NULL,
                      NULL,
                      NULL);
    //PRINTF("gaim_ic_set_cursor, [%d]\n", context->icid);
}

static void gaim_ic_set_client_widget(GtkIMContext *im_context,
#if GTK_CHECK_VERSION(4, 0, 0)
                                              GtkWidget *widget)
#else
                                              GdkWindow *widget)
#endif
{
    GAimIMContext *context = (GAimIMContext *)im_context;
    if (!widget) return;

#if GTK_CHECK_VERSION(4, 0, 0)
    gtk_im_context_set_client_widget(context->fallback, widget);

    if (context->client_widget != NULL)
        g_object_unref(context->client_widget);

    context->client_widget = widget;
    g_object_ref(context->client_widget);
#else
    gtk_im_context_set_client_window(context->fallback, widget);

    if (context->client_window != NULL)
        g_object_unref(context->client_window);
    context->client_window = widget;
    g_object_ref(context->client_window);
#endif
    PRINTF("gaim_ic_set_client_widget, [%d]\n",context->icid);
}

static void gaim_ic_get_preedit_string(GtkIMContext *im_context,
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
    //PRINTF("gaim_ic_get_preedit_string\n");
}

// Close widget
static void gaim_ic_finalize(GObject *object)
{
    GAimIMContext *context = (GAimIMContext *)object;

    if (context->connected) {
        g_dbus_proxy_call(context->context_proxy,
                          "Destroy",
                          g_variant_new("(u)", context->icid),
                          G_DBUS_CALL_FLAGS_NONE,
                          -1,
                          NULL,
                          NULL,
                          NULL);
    }

#if GTK_CHECK_VERSION(4, 0, 0)
    if (context->client_widget != NULL) {
        g_object_unref(context->client_widget);
        context->client_widget = NULL;
    }
#else
    if (context->client_window != NULL) {
        g_object_unref(context->client_window);
        context->client_window = NULL;
    }
#endif

    if (context->fallback != NULL) {
        g_object_unref(context->fallback);
        context->fallback = NULL;
    }

    ic_disconnect(context);
    G_OBJECT_CLASS(gaim_im_context_parent_class)->finalize(object);
}

/* A ic class against a text widget
 * A module against a gtk app.
 */
static void gaim_im_context_class_init(GAimIMContextClass *klass)
{
    GObjectClass *objclass = G_OBJECT_CLASS(klass);
    GtkIMContextClass *icclass = GTK_IM_CONTEXT_CLASS(klass);

    objclass->finalize = gaim_ic_finalize;
    icclass->filter_keypress = gaim_ic_filter_keypress;
    icclass->focus_in = gaim_ic_focus_in;
    icclass->focus_out = gaim_ic_focus_out;
    icclass->reset = gaim_ic_reset;
    icclass->set_cursor_location = gaim_ic_set_cursor_location;
#if GTK_CHECK_VERSION(4, 0, 0)
    icclass->set_client_widget = gaim_ic_set_client_widget;
#else
    icclass->set_client_window = gaim_ic_set_client_widget;
#endif
    icclass->get_preedit_string = gaim_ic_get_preedit_string;
}

static void gaim_im_context_class_finalize(GAimIMContextClass *klass)
{
}

static void gaim_im_context_init(GAimIMContext *context)
{
    context->connection = NULL;
    context->bus_proxy = NULL;
    context->context_proxy = NULL;
    context->fallback = gtk_im_context_simple_new();
#if GTK_CHECK_VERSION(4, 0, 0)
    context->client_widget = NULL;
#else
    context->client_window = NULL;
#endif
    context->icid = 0;
    context->connected = FALSE;

    g_signal_connect(context->fallback,
                     "commit",
                     G_CALLBACK(gaim_ic_on_fallback_commit),
                     context);

    ic_connect(context);
}

#if GTK_CHECK_VERSION(4, 0, 0)
G_MODULE_EXPORT void g_io_im_alpha_load(GIOModule *module)
{
    if (g_type_module_use(G_TYPE_MODULE(module))) {
        gaim_im_context_register_type(G_TYPE_MODULE(module));
        g_io_extension_point_implement(GTK_IM_MODULE_EXTENSION_POINT_NAME,
                                       gaim_im_context_get_type(),
                                       AIM_GTK_CONTEXT_ID,
                                       10);
    }
}

G_MODULE_EXPORT void g_io_im_alpha_unload(GIOModule *module)
{
    g_type_module_unuse(G_TYPE_MODULE(module));
}

G_MODULE_EXPORT gchar **g_io_im_alpha_query(void)
{
    gchar **extension_points = g_new0(gchar *, 2);

    extension_points[0] = g_strdup(GTK_IM_MODULE_EXTENSION_POINT_NAME);

    return extension_points;
}
#else
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
#endif
