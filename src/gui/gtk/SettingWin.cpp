#include <gtk/gtk.h>
#include <string>

#include "SettingWin.h"

G_DEFINE_TYPE(SettingWin, setting_win, GTK_TYPE_WINDOW);

#define ITEM_H  80
#define ITEM_W  240

#define ITEM_MAX 2

static const gchar* SettingMenuStr[] = {
    "用户词组",
    "快捷键设置",
};

static void setting_win_init(SettingWin *win)
{
}

static void setting_win_class_init(SettingWinClass *klass)
{

}

static void on_menuItem_clicked (GtkToolButton *toolbutton, gpointer user_data)
{
    printf("on_menuItem_clicked\n");
}

SettingWin *setting_win_new(GtkApplication *gtkapp, int x, int y)
{
    SettingWin* settingwin = (SettingWin *)g_object_new (SETTING_WIN_TYPE,
                                          "type", GTK_WINDOW_POPUP,
                                          "decorated", FALSE,
                                          "resizable", FALSE,
                                          "window-position",GTK_WIN_POS_MOUSE,
                                          "accept-focus", FALSE,
                                          NULL);
    SettingWinClass *klass = SETTING_WIN_GET_CLASS(settingwin);
    GtkWidget *win = ( GtkWidget *)(GTK_WINDOW(settingwin));
    //gtk_widget_set_size_request(win, ITEM_W, ITEM_H * ITEM_MAX);
#if 0
    GtkWidget *list_box = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER (win), list_box);

    for(int i=0; i<ITEM_MAX; i++) {
        GtkToolItem *item = gtk_tool_button_new (NULL, SettingMenuStr[i]);
        g_signal_connect(item, "clicked", G_CALLBACK (on_menuItem_clicked), NULL);
        gtk_container_add (GTK_CONTAINER (list_box), GTK_WIDGET (item));
    }
#endif
#if 0
    GtkBuilder* builder = gtk_builder_new_from_resource ("/org/gtk/aimapp/gui/gtk/setting.ui");
    GtkWidget* list_box = GTK_WIDGET (gtk_builder_get_object (builder, "list_box"));
    gtk_container_add(GTK_CONTAINER (win), list_box);
    GtkBuilder *g_object_unref (builder);
#endif
    GtkBuilder* builder = gtk_builder_new_from_resource ("/org/gtk/aimapp/gui/gtk/menu.ui");
    GMenuModel* menu_model = G_MENU_MODEL (gtk_builder_get_object (builder, "setting"));
    GtkMenu * menu = GtkMenu (gtk_menu_new_from_model (menu_model));
    GtkBuilder *g_object_unref (builder);

    //klass->x = x;
    //klass->y = y;
    //gtk_widget_show_all(GTK_WIDGET (win));
    return settingwin;
}

void setting_win_show_hide(SettingWin *win)
{
    SettingWinClass *klass = SETTING_WIN_GET_CLASS(win);

        printf("show setting win1\n");
    if (gtk_widget_get_mapped(GTK_WIDGET (win))) {
        gtk_widget_hide(GTK_WIDGET (win));
    } else {
        gtk_widget_show_all(GTK_WIDGET (win));
        printf("show setting win\n");
        GdkWindow *gdk_win = gtk_widget_get_window(GTK_WIDGET (win));
        gdk_window_move(gdk_win,  klass->x,  klass->y);
    }
}
void setting_win_hide(SettingWin *win)
{
    SettingWinClass *klass = SETTING_WIN_GET_CLASS(win);
    gtk_widget_hide(GTK_WIDGET (win));
}
