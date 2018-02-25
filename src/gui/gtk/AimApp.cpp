#include <string>
#include <vector>

#include <syslog.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <libayatana-appindicator/app-indicator.h>


#include "AimApp.h"
#include "AimWin.h"
#include "IcWin.h"
#include "Util.h"
#include "SettingMenu.h"

std::string  g_system_dir;


//void (*gui_activate_callback)(Display *dsy);
static gboolean aim_app_on_hide_imwin(gpointer user_data);

AimApp *aim_app_instance = NULL;

G_DEFINE_TYPE(AimApp, aim_app, GTK_TYPE_APPLICATION);


enum AimTrayIcon
{
    AIM_TRAY_ICON_APP,
    AIM_TRAY_ICON_CN,
    AIM_TRAY_ICON_EN,
};

static const gchar *aim_app_tray_icon_name(AimTrayIcon icon)
{
    switch (icon) {
    case AIM_TRAY_ICON_CN:
        return "icon-cn";
    case AIM_TRAY_ICON_EN:
        return "icon-en";
    case AIM_TRAY_ICON_APP:
    default:
        return "icon";
    }
}

static void aim_app_set_tray_icon(AimAppClass *klass, AimTrayIcon icon)
{
    app_indicator_set_icon_full(klass->systray, aim_app_tray_icon_name(icon), "AlphaIM");
}

static void aim_app_on_systray_popup(AppIndicator *systray,
               guint          button,
               guint          activate_time,
               gpointer       user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
#if GTK_CHECK_VERSION(3, 22, 0)
    gtk_menu_popup_at_pointer(klass->setmenu, NULL);
#else
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_menu_popup (klass->setmenu, NULL, NULL, NULL, NULL, button, activate_time);
    G_GNUC_END_IGNORE_DEPRECATIONS
#endif
}

static void aim_app_systray_connect_popup(AimAppClass *klass)
{
    if (g_signal_lookup("popup-menu", G_OBJECT_TYPE(klass->systray)) == 0)
        return;

    g_signal_connect(klass->systray, "popup-menu", G_CALLBACK(aim_app_on_systray_popup), NULL);
}

static gboolean aim_app_on_active_im(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);

    if (klass->bshow_imwin) {
        aim_win_enable_im(klass->imwin, true);
    }

    aim_app_set_tray_icon(klass, AIM_TRAY_ICON_CN);

    return false;
}

static gboolean aim_app_on_disactive_im(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);

    gtk_widget_hide(GTK_WIDGET (klass->icwin));

    aim_win_enable_im(klass->imwin, false);

    if (klass->hpwin != NULL) {
        help_win_hide(klass->hpwin);
    }

    aim_app_set_tray_icon(klass, AIM_TRAY_ICON_APP);

    return false;
}

static gboolean aim_app_on_switch_lan(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    bool is_cn = *((gboolean *)user_data);

    if (is_cn)
        aim_app_set_tray_icon(klass, AIM_TRAY_ICON_CN);
    else
        aim_app_set_tray_icon(klass, AIM_TRAY_ICON_EN);

    aim_win_switch_lan(klass->imwin, is_cn);

    g_free(user_data);
    return false;
}

static gboolean aim_app_on_switch_pun(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);

    aim_win_switch_pun(klass->imwin, *((gboolean *)user_data));

    g_free(user_data);
    return false;
}

static gboolean aim_app_on_show_icwin(gpointer user_data)
{
    GVariant *parameters = (GVariant*)user_data;
    gint32 x, y, w, h;
    gchar *input;
    gchar *items;

    g_variant_get(parameters, "(iiiiiss)", NULL, &x, &y, &w, &h, &input, &items);

    PRINTF("aim_app_on_show_icwin %d, %d ,%d ,%d\n", x,y,w,h);

    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    ic_win_refresh(klass->icwin, x, y, w, h, input, items);

    g_free(input);
    g_free(items);

    g_variant_unref(parameters);
    return false;
}

static gboolean aim_app_on_hide_icwin(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    gtk_widget_hide(GTK_WIDGET (klass->icwin));

    return false;
}

static gboolean aim_app_on_hide_imwin(gpointer user_data)
{
    AimWin *imwin = (AimWin *)user_data;
    gtk_widget_hide(GTK_WIDGET (imwin));
    return false; // Called only once.
    //return true; // Called every interval.
}


static void aim_app_message_action(int action)
{
    //printf("aim_app_message_action: action:%d\n", action);
    switch (action) {
    case MSG_IM_ON: {
        //g_signal_emit(data, klass->active_im_id, 0);
        gdk_threads_add_idle(aim_app_on_active_im, NULL);
        break;
    }

    case MSG_IM_OFF: {
        gdk_threads_add_idle(aim_app_on_disactive_im, NULL);
        break;
    }

    case MSG_IM_CLOSE: {
        gdk_threads_add_idle(aim_app_on_hide_icwin, NULL);
        break;
    }

    case MSG_IM_COMMIT: {
        gdk_threads_add_idle(aim_app_on_hide_icwin, NULL);
        break;
    }

    case MSG_IM_CN: {
        gboolean *is_cn = (gboolean *) g_malloc(sizeof(gboolean));
        *is_cn = true;
        gdk_threads_add_idle(aim_app_on_switch_lan, is_cn);

        break;
    }

    case MSG_IM_EN: {
        gboolean *is_cn = (gboolean *) g_malloc(sizeof(gboolean));
        *is_cn = false;
        gdk_threads_add_idle(aim_app_on_switch_lan, is_cn);

        break;
    }

    case MSG_IM_CPUN: {
        gboolean *is_cn = (gboolean *) g_malloc(sizeof(gboolean));
        *is_cn = true;
        gdk_threads_add_idle(aim_app_on_switch_pun, is_cn);
        break;
    }

    case MSG_IM_EPUN: {
        gboolean *is_cn = (gboolean *) g_malloc(sizeof(gboolean));
        *is_cn = false;
        gdk_threads_add_idle(aim_app_on_switch_pun, is_cn);
        break;
    }
    default:
        break;
    }
}

static void aim_app_on_signal (GDBusProxy *proxy,
                               gchar      *sender_name,
                               gchar      *signal_name,
                               GVariant   *parameters,
                               gpointer    user_data)
{
    if (g_variant_check_format_string(parameters, "(iiiiiss)", FALSE)) {
        int i1, i2, i3, i4, i5;
        gchar *s1;
        gchar *s2;
        g_variant_get(parameters, "(iiiiiss)", &i1, &i2, &i3, &i4, &i5, &s1, &s2);

        GVariant *user_data = g_variant_new ("(iiiiiss)", i1, i2, i3, i4, i5, s1, s2);
        gdk_threads_add_idle(aim_app_on_show_icwin, (gpointer) user_data);

        g_free(s1);
        g_free(s2);
    } else {
        gint32 msgid;
        g_variant_get (parameters, "(i)", &msgid);
        aim_app_message_action(msgid);
    }
}

static bool aim_app_dbus_init()
{
    GError *error = NULL;
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);

    klass->event_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                         G_DBUS_PROXY_FLAGS_NONE,
                                         NULL, /* GDBusInterfaceInfo */
                                         AIM_SRV_NAME,
                                         AIM_NOTIFY_PATH,
                                         AIM_NOTIFY_INTF,
                                         NULL, /* GCancellable */
                                         &error);
    if (klass->event_proxy == NULL) {
        //g_printerr ("Error creating event proxy: %s\n", error->message);
        syslog (LOG_ERR, "Error creating event proxy %s \n", error->message);
        g_error_free (error);
        return false;
    }
    error = NULL;
    g_signal_connect (klass->event_proxy,
                      "g-signal",
                      G_CALLBACK (aim_app_on_signal),
                      NULL);

    klass->im_proxy = g_dbus_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION,
                                        G_DBUS_PROXY_FLAGS_NONE,
                                        NULL,
                                        AIM_SRV_NAME,
                                        AIM_SRV_PATH,
                                        AIM_SRV_INTF,
                                        NULL,
                                        &error);

    if (klass->im_proxy == NULL) {
        //g_printerr ("Error creating im proxy: %s\n", error->message);
        syslog (LOG_ERR, "Error creating im proxy: %s\n", error->message);
        g_error_free (error);
        return false;
    }
    syslog (LOG_INFO, "dbus init success.\n");
    return true;
}

static void aim_app_init (AimApp *app)
{
    g_system_dir  = Util::execDir();
    g_system_dir +=  "/system";
    if (!Util::isDirExist(g_system_dir))
        g_system_dir = DATADIR;
}

#if 0
static void aim_app_startup(GApplication *app)
{
    G_APPLICATION_CLASS (aim_app_parent_class)->startup (app);
}
#endif

static void aim_app_dispose(GObject *gobject)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(gobject);
}

static void aim_app_finalize(GObject *gobject)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(gobject);

    if (klass->systray != NULL) {
        g_object_unref(klass->systray);
        klass->systray = NULL;
    }

    g_free(klass->tray_icon_theme_path);
    g_free(klass->tray_icon_app_path);
    g_free(klass->tray_icon_en_path);
    g_free(klass->tray_icon_cn_path);
    klass->tray_icon_theme_path = NULL;
    klass->tray_icon_app_path = NULL;
    klass->tray_icon_en_path = NULL;
    klass->tray_icon_cn_path = NULL;

    G_OBJECT_CLASS(aim_app_parent_class)->finalize(gobject);
}

static void aim_app_open(GApplication  *app,
                  GFile        **files,
                  gint          n_files,
                  const gchar   *hint)
{

}

static void aim_app_activate (GApplication *app)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(app);
    GtkApplication *gtkapp = GTK_APPLICATION (app);
#if GTK_CHECK_VERSION(3, 22, 0)
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = display != NULL ? gdk_display_get_primary_monitor(display) : NULL;
    GdkRectangle workarea = {0, 0, 0, 0};
    if (monitor != NULL) {
        gdk_monitor_get_workarea(monitor, &workarea);
    }
    klass->x = workarea.x + workarea.width - 280;
    klass->y = workarea.y + workarea.height - 80;
#else
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    GdkScreen *gdk_screen = gdk_screen_get_default();
    klass->x = gdk_screen_get_width(gdk_screen) - 280;
    klass->y = gdk_screen_get_height(gdk_screen) - 80;
    G_GNUC_END_IGNORE_DEPRECATIONS
#endif

    IcWin *icwin = ic_win_new();
    klass->icwin = icwin;
    gtk_application_add_window(gtkapp, GTK_WINDOW (icwin));


    AimWin *imwin = aim_win_new(klass->x, klass->y);
    klass->imwin = imwin;
    gtk_application_add_window(gtkapp, GTK_WINDOW (imwin));

    klass->hpwin = NULL;

    std::string icons_path = g_system_dir + "/" +  ICONS_PATH;
    klass->tray_icon_theme_path = g_strdup(icons_path.c_str());
    klass->tray_icon_app_path = g_strdup((g_system_dir + "/"  + "app.png").c_str());
    klass->tray_icon_en_path = g_strdup((icons_path + "/en.png").c_str());
    klass->tray_icon_cn_path = g_strdup((icons_path + "/cn.png").c_str());

    klass->setmenu = setting_menu_new(app);

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    klass->systray = app_indicator_new("org.gtk.aimapp",
                                       aim_app_tray_icon_name(AIM_TRAY_ICON_APP),
                                       APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    G_GNUC_END_IGNORE_DEPRECATIONS
    app_indicator_set_icon_theme_path(klass->systray, klass->tray_icon_theme_path);
    app_indicator_set_menu(klass->systray, klass->setmenu);
    app_indicator_set_status(klass->systray, APP_INDICATOR_STATUS_ACTIVE);
    aim_app_systray_connect_popup(klass);
    klass->bshow_imwin = true;
    aim_app_dbus_init();
}

static void aim_app_class_init(AimAppClass *klass)
{
    //G_APPLICATION_CLASS (klass)->startup  = aim_app_startup;
    G_APPLICATION_CLASS (klass)->activate = aim_app_activate;
    G_APPLICATION_CLASS (klass)->open     = aim_app_open;
    G_OBJECT_CLASS      (klass)->dispose  = aim_app_dispose;
    G_OBJECT_CLASS      (klass)->finalize = aim_app_finalize;
}



AimApp* aim_app_new(void)
{
    //app = gtk_application_new ("org.gnome.example", G_APPLICATION_FLAGS_NONE);
    return  (AimApp*)g_object_new(AIM_APP_TYPE,
                                 "application-id", "org.gtk.aimapp2",
                                 NULL);
}



gboolean aim_app_on_show_hpwin(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    if (klass->hpwin == NULL) {
        GtkApplication *gtkapp = GTK_APPLICATION(aim_app_instance);
        klass->hpwin = help_win_new(klass->x, klass->y);

        gtk_application_add_window(gtkapp, GTK_WINDOW (klass->hpwin));
    }

    help_win_show_hide(klass->hpwin);

    return false;
}

gboolean aim_app_on_hide_hpwin(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    if (klass->hpwin != NULL) {
         help_win_hide(klass->hpwin);
    }
    return false;
}

void aim_app_message_send(int action)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    g_dbus_proxy_call_sync (klass->im_proxy, "GuiMessage",
                            g_variant_new ("(i)", action), G_DBUS_CALL_FLAGS_NONE, -1, NULL, NULL);
}


int main(int argc, char* argv[])
{
    aim_app_instance = aim_app_new();

    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);


    int status = g_application_run(G_APPLICATION(GTK_APPLICATION(aim_app_instance)), argc, argv);

    g_object_unref (klass->event_proxy);
    g_object_unref (klass->im_proxy);
    g_object_unref(aim_app_instance);

    return status;
}
