#include <string>
#include <vector>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#ifdef HAS_APPINDICATOR3
#include <libappindicator/app-indicator.h>
#endif

#include "AimApp.h"
#include "AimWin.h"
#include "IcWin.h"

void (*gui_activate_callback)(Display *dsy);
static gboolean aim_app_on_hide_imwin(gpointer user_data);

AimApp *aim_app_instance = NULL;

G_DEFINE_TYPE(AimApp, aim_app, GTK_TYPE_APPLICATION);

static void aim_app_on_action_systray_press(GSimpleAction *action,
                                            GVariant      *parameter,
                                            gpointer       user_data)
{
    //printf("joni aim_app_on_action_systray_press \n");
    //AimWin *imwin = (AimWin *)user_data;
    //aim_win_show_hide(imwin);

    //AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    //if (klass->hpwin != NULL) {
    //     help_win_hide(klass->hpwin);
    //}
}

static GActionEntry app_actions[] = {
    { "systray-press", aim_app_on_action_systray_press, NULL, NULL, NULL }
};

static void aim_app_init (AimApp *app)
{
}

static void aim_app_startup(GApplication *app)
{
    G_APPLICATION_CLASS (aim_app_parent_class)->startup (app);
    g_action_map_add_action_entries (G_ACTION_MAP (app),
                                     app_actions, G_N_ELEMENTS (app_actions),
                                     app);
}

gboolean aim_app_on_systray_press(GtkStatusIcon *status_icon,
                                  GdkEvent      *event,
                                  gpointer       user_data)
{
    AimWin *imwin = (AimWin *)user_data;
    aim_win_show_hide(imwin);

    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    if (klass->hpwin != NULL) {
         help_win_hide(klass->hpwin);
    }

    return false;
}


static void aim_app_dispose(GObject *gobject)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(gobject);

    gui_messager_abort(klass->gui_messager);
    g_object_unref(klass->gui_messager);
}

static void aim_app_finalize(GObject *gobject)
{
printf("aim_app_finalize\n");
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
printf("aim_app_activate\n");
    GdkScreen *gdk_screen = gdk_screen_get_default ();
    gint screen_w = gdk_screen_get_width(gdk_screen);
    gint screen_h = gdk_screen_get_height(gdk_screen);

    IcWin *icwin = ic_win_new();
    klass->icwin = icwin;
    gtk_application_add_window(gtkapp, GTK_WINDOW (icwin));

    int x = screen_w - 280;
    int y = screen_h - 80;
    klass->x = x;
    klass->y = y;

    AimWin *imwin = aim_win_new(x, y);
    klass->imwin = imwin;
    gtk_application_add_window(gtkapp, GTK_WINDOW (imwin));

    //klass->hpwin = help_win_new(x, y);
    //gtk_application_add_window(gtkapp, GTK_WINDOW (klass->hpwin));
    klass->hpwin = NULL;
    //GdkWindow *gdk_imwin = gtk_widget_get_window(GTK_WIDGET (imwin));
    //Window   xid = gdk_x11_window_get_xid(gdk_imwin);  
    Display *dsy = gdk_x11_get_default_xdisplay();

    //GtkIconTheme *icons = gtk_icon_theme_get_default();
    //gtk_icon_theme_append_search_path(icons, "system/theme");
    //printf ("joni debug has icon: %d\n", gtk_icon_theme_has_icon(icons, "setting"));

    klass->systray_img_app =  gtk_image_new_from_file((system_dir + "/"  + "app.png").c_str());
    std::string icons_path = system_dir + "/" +  ICONS_PATH;
    klass->systray_img_en =  gtk_image_new_from_file((icons_path + "/en.png").c_str());
    klass->systray_img_cn =  gtk_image_new_from_file((icons_path + "/cn.png").c_str());

    klass->bshow_imwin = SHOW_IMWIN;

#ifdef HAS_NOTIFICATION
    GNotification *notification = g_notification_new ("aim");
    GFile *file = g_file_new_for_path ((system_dir + "/"  + "app.png").c_str());
    GIcon *icon = g_file_icon_new (file);
    g_notification_set_icon (notification,G_ICON (icon));
    g_object_unref (icon);
    g_object_unref (file);
	g_application_send_notification (app, "app-systray-press", notification);
    g_notification_add_button_with_target (notification, "label", "app.systray-press", "s", "bla");
    g_notification_set_default_action (notification, "app.systray-press");
    //g_notification_add_button (notification, "5 minutes", "app.reply-5-minutes");

    g_object_unref(notification);
#elif defined(HAS_APPINDICATOR3)
    /* Indicator */
    AppIndicator *indicator = app_indicator_new ("org.gtk.aimapp",
                                   "virtualbox",
                                    APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_attention_icon (indicator, "virtualbox");
    app_indicator_set_icon(indicator, "virtualbox");
     g_object_unref(indicator);
#else
    klass->systray = gtk_status_icon_new_from_pixbuf(gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_app)));
    g_signal_connect(GTK_STATUS_ICON (klass->systray), "button-press-event", G_CALLBACK (aim_app_on_systray_press), klass->imwin);
#endif
    //gdk_threads_add_timeout_seconds (3, aim_app_on_hide_imwin, imwin);
    gui_activate_callback(dsy);
}

static void aim_app_class_init(AimAppClass *klass)
{
    G_APPLICATION_CLASS (klass)->startup  = aim_app_startup;
    G_APPLICATION_CLASS (klass)->activate = aim_app_activate;
    G_APPLICATION_CLASS (klass)->open     = aim_app_open;
    G_OBJECT_CLASS      (klass)->dispose  = aim_app_dispose;
    G_OBJECT_CLASS      (klass)->finalize = aim_app_finalize;
}

gboolean aim_app_on_active_im(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);

    if (klass->bshow_imwin) {
        aim_win_enable_im(klass->imwin, true);
    } else {
        gtk_status_icon_set_from_pixbuf(klass->systray, gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_cn)));
        //gtk_status_icon_set_from_pixbuf(klass->systray, gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_app)));
    }
    return false;
}

gboolean aim_app_on_disactive_im(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    gtk_status_icon_set_from_pixbuf(klass->systray, gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_app)));

    gtk_widget_hide(GTK_WIDGET (klass->icwin));

    aim_win_enable_im(klass->imwin, false);

    if (klass->hpwin != NULL) {
         help_win_hide(klass->hpwin);
    }

    return false;
}

gboolean aim_app_on_switch_lan(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);

    bool is_cn = *((gboolean *)user_data);
    if (!gtk_widget_get_mapped(GTK_WIDGET (klass->imwin))) {
    	if (is_cn)
            gtk_status_icon_set_from_pixbuf(klass->systray, gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_cn)));
    	else
            gtk_status_icon_set_from_pixbuf(klass->systray, gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_en)));
    }
    aim_win_switch_lan(klass->imwin, is_cn);
    return false;
}

gboolean aim_app_on_switch_pun(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);

    aim_win_switch_pun(klass->imwin, *((gboolean *)user_data));
    return false;
}

gboolean aim_app_on_show_icwin(gpointer user_data)
{
    Message *pmsg = (Message*)user_data;
    int x = pmsg->iArg1;
    int y = pmsg->iArg2;
    int w = (int)pmsg->fArg1;
    int h = (int)pmsg->fArg2;
    std::string strInput = pmsg->strArg1;
    std::deque<IMItem> *items = (std::deque<IMItem> *)(pmsg->pArg1);

    //PRINTF("aim_app_on_show_icwin %d, %d ,%d ,%d\n", x,y,w,h);
    //printf("aim_app_on_show_icwin %d, %d ,%d ,%d\n", x,y,w,h);

    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    ic_win_refresh(klass->icwin, x, y, w, h, strInput, items);

    //delete items;
    delete pmsg;

    return false;
}

gboolean aim_app_on_hide_icwin(gpointer user_data)
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

AimApp* aim_app_new(void)
{
    //app = gtk_application_new ("org.gnome.example", G_APPLICATION_FLAGS_NONE);
    return (AimApp*)g_object_new(AIM_APP_TYPE,
                                 "application-id", "org.gtk.aimapp",
                                 NULL);
}

int aim_app_main(MessageQueue *q, fun_gui_activate_callback cb, int argc, char* argv[])
{
    gui_activate_callback = cb;

    aim_app_instance = aim_app_new();

    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    klass->gui_messager = gui_messger_new(q);

    int status = g_application_run(G_APPLICATION(GTK_APPLICATION(aim_app_instance)), argc, argv);
    g_object_unref(aim_app_instance);
    return status;
}
