#include <string>
#include <vector>

#include <syslog.h>  //cat /var/log/syslog
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#ifdef HAS_APPINDICATOR3
#include <libappindicator/app-indicator.h>
#endif

#include "AimApp.h"
#include "AimWin.h"
#include "IcWin.h"
#include "Util.h"

std::string  g_system_dir;


//void (*gui_activate_callback)(Display *dsy);
static gboolean aim_app_on_hide_imwin(gpointer user_data);

AimApp *aim_app_instance = NULL;

G_DEFINE_TYPE(AimApp, aim_app, GTK_TYPE_APPLICATION);

static gboolean aim_app_on_systray_press(GtkStatusIcon *status_icon,
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

static gboolean aim_app_on_active_im(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);

    if (klass->bshow_imwin) {
        aim_win_enable_im(klass->imwin, true);
    }

	gtk_status_icon_set_from_pixbuf(klass->systray, gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_cn)));

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

	gtk_status_icon_set_from_pixbuf(klass->systray, gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_app)));

    return false;
}

static gboolean aim_app_on_switch_lan(gpointer user_data)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(aim_app_instance);
    bool is_cn = *((gboolean *)user_data);	

	if (is_cn)
		gtk_status_icon_set_from_pixbuf(klass->systray, gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_cn)));
	else
		gtk_status_icon_set_from_pixbuf(klass->systray, gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_en)));

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

    //gui_messager_abort(klass->gui_messager);
    //g_object_unref(klass->gui_messager);
}

static void aim_app_finalize(GObject *gobject)
{
//printf("aim_app_finalize\n");
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
    GdkScreen *gdk_screen = gdk_screen_get_default ();
    klass->x = gdk_screen_get_width(gdk_screen) - 280;
    klass->y = gdk_screen_get_height(gdk_screen) - 80;

    IcWin *icwin = ic_win_new();
    klass->icwin = icwin;
    gtk_application_add_window(gtkapp, GTK_WINDOW (icwin));


    AimWin *imwin = aim_win_new(klass->x, klass->y);
    klass->imwin = imwin;
    gtk_application_add_window(gtkapp, GTK_WINDOW (imwin));

    klass->hpwin = NULL;

    //GtkIconTheme *icons = gtk_icon_theme_get_default();
    //gtk_icon_theme_append_search_path(icons, "system/theme");
    //printf ("joni debug has icon: %d\n", gtk_icon_theme_has_icon(icons, "setting"));

#if 1
    klass->systray_img_app =  gtk_image_new_from_file((g_system_dir + "/"  + "app.png").c_str());
    std::string icons_path = g_system_dir + "/" +  ICONS_PATH;
    klass->systray_img_en =  gtk_image_new_from_file((icons_path + "/en.png").c_str());
    klass->systray_img_cn =  gtk_image_new_from_file((icons_path + "/cn.png").c_str());
    klass->systray = gtk_status_icon_new_from_pixbuf(gtk_image_get_pixbuf(GTK_IMAGE (klass->systray_img_app)));
    //g_signal_connect(GTK_STATUS_ICON (klass->systray), "button-press-event", G_CALLBACK (aim_app_on_systray_press), klass->imwin);
#endif
    klass->bshow_imwin = SHOW_IMWIN;

#ifdef HAS_APPINDICATOR3
    /* Indicator */
    AppIndicator *indicator = app_indicator_new ("org.gtk.aimapp",
                                   "virtualbox",
                                    APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_attention_icon (indicator, "virtualbox");
    app_indicator_set_icon(indicator, "virtualbox");
	g_object_unref(indicator);
#endif

#if 0
	if ( klass->bshow_imwin) {
		aim_win_show_hide(klass->imwin);
		//gdk_threads_add_timeout_seconds (5, aim_app_on_hide_imwin, imwin);
		}
#endif

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
                                 "application-id", "org.gtk.aimapp",
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

