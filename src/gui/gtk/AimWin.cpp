#include <gtk/gtk.h>
#include <string>

#include "Application.h"
#include "AimWin.h"
#include "aim.h"

G_DEFINE_TYPE(AimWin, aim_win, GTK_TYPE_WINDOW /*GTK_TYPE_APPLICATION_WINDOW*/);

extern gboolean aim_app_on_hide_hpwin(gpointer user_data);

static void aim_win_init(AimWin *win)
{
}

static void aim_win_class_init(AimWinClass *klass)
{
}

static void on_lanbutton_clicked (GtkButton *button, gpointer user_data)
{
    gApp->pSysMsgQ->push(MSG_UI_LAN);

    gdk_threads_add_idle(aim_app_on_hide_hpwin, user_data);
}

static void on_punbutton_clicked (GtkButton *button, gpointer user_data)
{
    gApp->pSysMsgQ->push(MSG_UI_PUN);

    gdk_threads_add_idle(aim_app_on_hide_hpwin, user_data);
}

extern gboolean aim_app_on_show_hpwin(gpointer user_data);
static void on_helpbutton_clicked (GtkButton *button, gpointer user_data)
{
    gdk_threads_add_idle(aim_app_on_show_hpwin, user_data);
}

void aim_win_set_pos(AimWin *win, int x, int y)
{
    AimWinClass *klass = AIM_WIN_GET_CLASS(win);
    klass->x = x;
    klass->y = y;
}

void aim_win_show_hide(AimWin *win)
{
    AimWinClass *klass = AIM_WIN_GET_CLASS(win);
    if (gtk_widget_get_mapped(GTK_WIDGET (win))) {
        gtk_widget_hide(GTK_WIDGET (win));
    } else {
        gtk_widget_show_all(GTK_WIDGET (win));

        GdkWindow *gdk_win = gtk_widget_get_window(GTK_WIDGET (win));
        gdk_window_move(gdk_win,  klass->x,  klass->y);

        //printf("aim_win_show_hide, %d, %d\n",klass->x,  klass->y);
        //gtk_window_present(GTK_WINDOW (imwin));
    }
}

void aim_win_enable_im(AimWin *win, bool en)
{
    AimWinClass *klass = AIM_WIN_GET_CLASS(win);

    if (en) {
        gtk_widget_show_all(GTK_WIDGET (win));

        GdkWindow *gdk_win = gtk_widget_get_window(GTK_WIDGET (win));
        gdk_window_move(gdk_win,  klass->x,  klass->y);

        aim_win_switch_lan(win, true);

        //printf("aim_win_enable_im, %d, %d\n",klass->x,  klass->y);
        //if (gtk_widget_get_mapped(GTK_WIDGET (win))) {
        //gtk_widget_set_sensitive(klass->lan_button, en);
       // }
    } else {
        //printf("aim_win_disable_im\n");
        aim_win_switch_lan(win, false);

        gtk_widget_hide(GTK_WIDGET (win));
    }
}

void aim_win_switch_pun(AimWin *win, bool is_cn)
{
    AimWinClass *klass = AIM_WIN_GET_CLASS(win);
    std::string icons_path = system_dir + "/" +  ICONS_PATH;

    if (gtk_widget_get_mapped(GTK_WIDGET (win))) {
        if (is_cn) {
            GtkWidget *image_pun = gtk_image_new_from_file((icons_path + "/pun.png").c_str());
            gtk_button_set_image(GTK_BUTTON(klass->pun_button), (GtkWidget *) image_pun);
        } else {
            GtkWidget *image_epun = gtk_image_new_from_file((icons_path + "/epun.png").c_str());
            gtk_button_set_image(GTK_BUTTON(klass->pun_button), (GtkWidget *) image_epun);
        }
    }
}

void aim_win_switch_lan(AimWin *win, bool is_cn)
{
    AimWinClass *klass = AIM_WIN_GET_CLASS(win);
    std::string icons_path = system_dir + "/" +  ICONS_PATH;
    if (gtk_widget_get_mapped(GTK_WIDGET (win))) {
        if (is_cn) {
            GtkWidget *image_cn = gtk_image_new_from_file((icons_path + "/cn.png").c_str());
            gtk_button_set_image(GTK_BUTTON(klass->lan_button), (GtkWidget *) image_cn);
        } else {
            GtkWidget *image_en = gtk_image_new_from_file((icons_path + "/en.png").c_str());
            gtk_button_set_image(GTK_BUTTON(klass->lan_button), (GtkWidget *) image_en);
        }
    }
}

AimWin *aim_win_new(int x, int y)
{
    #define IMWIN_W  126
    #define IMWIN_H  40
    #define BTN_W    50
    std::string icons_path = system_dir + "/" +  ICONS_PATH;

    GtkWidget *button;
    GtkWidget *image;

    AimWin* imwin = (AimWin *)g_object_new (AIM_WIN_TYPE,
                                            "type",         GTK_WINDOW_POPUP,
                                            "decorated",    FALSE,
                                            "resizable",    FALSE,
                                            "accept-focus", FALSE,
                                            "gravity",      GDK_GRAVITY_SOUTH_EAST,
                                            NULL);

    AimWinClass *klass = AIM_WIN_GET_CLASS(imwin);

    GtkWidget *win = GTK_WIDGET (imwin);
    gtk_widget_set_size_request(win, IMWIN_W, IMWIN_H);

    GtkWidget *fixed_container =  gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER (win), fixed_container);

    //button = gtk_button_new_with_label ("汉");
    //gtk_widget_set_sensitive(button, false);
    GtkWidget *image_cn = gtk_image_new_from_file((icons_path + "/en.png").c_str());
    button = gtk_button_new();
    button = (GtkWidget *) g_object_new(GTK_TYPE_BUTTON,
                                        "use-underline", TRUE,
                                        "relief", GTK_RELIEF_NONE,
                                        NULL);

    //GtkStyleContext  *style = gtk_widget_get_style_context(button);
    //gtk_style_context_add_class (style, "image-button");
    gtk_button_set_image(GTK_BUTTON(button), (GtkWidget *) image_cn);
    gtk_widget_set_size_request(button, 40, 40);
    gtk_fixed_put((GtkFixed *)fixed_container, button, 1, 1);
    klass->lan_button = button;
    g_signal_connect(button, "clicked", G_CALLBACK (on_lanbutton_clicked), NULL);

    GtkWidget *image_pun  = gtk_image_new_from_file((icons_path + "/pun.png").c_str());
    button = (GtkWidget *) g_object_new(GTK_TYPE_BUTTON,
                                        "use-underline", TRUE,
                                        "relief", GTK_RELIEF_NONE,
                                        NULL);
    gtk_button_set_image(GTK_BUTTON(button), (GtkWidget *)  image_pun);
    gtk_widget_set_size_request(button, 40, 40);
    gtk_fixed_put((GtkFixed *)fixed_container, button, 40, 1);
    klass->pun_button = button;
    g_signal_connect(button, "clicked", G_CALLBACK (on_punbutton_clicked), NULL);

    image = gtk_image_new_from_file((icons_path + "/help.png").c_str());
    button = (GtkWidget *) g_object_new(GTK_TYPE_BUTTON,
                                        "use-underline", TRUE,
                                        "relief", GTK_RELIEF_NONE,
                                        NULL);
    gtk_button_set_image(GTK_BUTTON(button), (GtkWidget *) image);
    gtk_widget_set_size_request(button, 40, 40);
    gtk_fixed_put((GtkFixed *)fixed_container, button, 80, 1);
    g_signal_connect(button, "clicked", G_CALLBACK (on_helpbutton_clicked), NULL);

#if 0
    image = gtk_image_new_from_file((icons_path + "/setting.png").c_str());
    button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(button), (GtkWidget *)image);
    gtk_widget_set_size_request(button, 40, 40);
    gtk_fixed_put((GtkFixed *)fixed_container, button, 120, 1);
#endif


#if 0
    gtk_widget_show_all (win);

    // Must after gtk_widget_show_all();
    GdkWindow *gdk_imwin = gtk_widget_get_window(win);
    gdk_window_move(gdk_imwin, x, y);
#endif
    klass->x = x;
    klass->y = y;
    //g_signal_connect (win, "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);

    return imwin;
}
