#include <gtk/gtk.h>

#include "AimApp.h"
#include "SettingMenu.h"
#include "aim.h"

extern gboolean aim_app_on_show_hpwin(gpointer user_data);

static void load_labels(SettingMenuState *state, GKeyFile *file)
{
    gsize count = 0;
    gchar **keys = g_key_file_get_keys(file, "Labels", &count, NULL);
    for (gsize index = 0; index < count; ++index) {
        gchar *value = g_key_file_get_string(file, "Labels", keys[index], NULL);
        if (value != NULL)
            g_hash_table_replace(state->labels, g_strdup(keys[index]), value);
    }
    g_strfreev(keys);
}

const gchar *setting_menu_label(SettingMenuState *state, const gchar *key)
{
    const gchar *value = static_cast<const gchar *>(g_hash_table_lookup(state->labels, key));
    return value != NULL ? value : key;
}

static void show_help(GtkMenuItem *item, gpointer user_data)
{
    aim_app_on_show_hpwin(user_data);
}

static void destroy_menu_state(gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    g_hash_table_unref(state->labels);
    if (state->settings_window != NULL)
        gtk_widget_destroy(state->settings_window);
    if (state->input_window != NULL)
        gtk_widget_destroy(state->input_window);
    g_free(state);
}

GtkMenu *setting_menu_new(GApplication *app)
{
    const gchar *labels[] = {"Help", "Input method settings", "Input and copy"};
    GCallback callbacks[] = {G_CALLBACK(show_help),
        G_CALLBACK(setting_window_show), G_CALLBACK(input_copy_window_show)};

    SettingMenuState *state = g_new0(SettingMenuState, 1);
    state->app = app;
    state->labels = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    GKeyFile *file = g_key_file_new();
    GBytes *data = g_resources_lookup_data("/org/gtk/aimapp/gui/gtk/labels.ini",
                                         G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);
    if (data != NULL) {
        gsize length = 0;
        const gchar *contents = static_cast<const gchar *>(g_bytes_get_data(data, &length));
        if (g_key_file_load_from_data(file, contents, length, G_KEY_FILE_NONE, NULL))
            load_labels(state, file);
        g_bytes_unref(data);
    }
    g_key_file_unref(file);
    file = g_key_file_new();
    gchar *filename = g_build_filename(g_get_user_config_dir(), IM_ID, "labels.ini", NULL);
    if (g_key_file_load_from_file(file, filename, G_KEY_FILE_NONE, NULL))
        load_labels(state, file);
    g_free(filename);
    g_key_file_unref(file);

    GtkWidget *menu = gtk_menu_new();
    g_object_set_data_full(G_OBJECT(menu), "settings-state", state, destroy_menu_state);
    for (guint index = 0; index < G_N_ELEMENTS(labels); ++index) {
        GtkWidget *item = gtk_menu_item_new_with_label(setting_menu_label(state, labels[index]));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
        g_signal_connect(item, "activate", callbacks[index], state);
    }
    gtk_widget_show_all(menu);
    return GTK_MENU(menu);
}
