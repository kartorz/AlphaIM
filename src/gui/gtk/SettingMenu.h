#ifndef _SETTINGMENU_H_
#define _SETTINGMENU_H_

#include <gtk/gtk.h>

struct SettingMenuState
{
    GApplication *app;
    GtkWidget *settings_window;
    GtkWidget *onoffkey_modifier;
    GtkWidget *imswitchkey_modifier;
    GtkWidget *ic_mode_per_app;
    GtkWidget *ic_mode_global;
    GtkWidget *learn_enabled;
    gboolean loading;
    GtkWidget *input_window;
    GtkWidget *input_entry;
    GHashTable *labels;
};

const gchar *setting_menu_label(SettingMenuState *state, const gchar *key);
void setting_window_show(GtkMenuItem *item, gpointer user_data);
void input_copy_window_show(GtkMenuItem *item, gpointer user_data);

extern GtkMenu* setting_menu_new(GApplication *app);

#endif
