#include <gtk/gtk.h>
#include <string>

static void preferences_activated (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
        printf("preferences_activated\n");
        //gtk_menu_popup (gtk_menu, NULL, NULL, NULL, NULL, 0, 0);
}

static void quit_activated (GSimpleAction *action,
                       GVariant      *parameter,
                       gpointer       app)
{
        printf("preferences_activated\n");
        //gtk_menu_popup (gtk_menu, NULL, NULL, NULL, NULL, 0, 0);
}


static GActionEntry app_entries[] =
{
  { "preferences", preferences_activated, NULL, NULL, NULL },
  { "quit", quit_activated, NULL, NULL, NULL }
};


static void on_menuItem_clicked (GtkToolButton *toolbutton, gpointer user_data)
{
    printf("on_menuItem_clicked\n");
}


GtkMenu* setting_menu_new(GApplication *app)
{
    printf("setting_menu_new\n");
    g_action_map_add_action_entries (G_ACTION_MAP (app),
                                   app_entries, G_N_ELEMENTS (app_entries),
                                   app);

    GtkBuilder* builder = gtk_builder_new_from_resource ("/org/gtk/aimapp/gui/gtk/menu.ui");
    GMenuModel* menu_model = G_MENU_MODEL (gtk_builder_get_object (builder, "setting"));
#if 0
    GObject *obj = gtk_builder_get_object (builder, "setting");
    if (obj == NULL)
            printf("can't get setting obj\n");
#endif
    GtkMenu* menu = GTK_MENU (gtk_menu_new_from_model (menu_model));
    g_object_unref (builder);
    return menu;
}

