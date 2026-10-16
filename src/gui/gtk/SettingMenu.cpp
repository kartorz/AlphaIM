#include <gtk/gtk.h>

#include "AimApp.h"
#include "SettingMenu.h"
#include "aim.h"

extern void aim_app_message_send(int action, int argument);

struct SettingMenuState
{
    GApplication *app;
    GtkWidget *shortcut_window;
    GtkWidget *shortcut_enabled;
    GtkWidget *shortcut_modifier;
    GtkWidget *input_window;
    GtkWidget *input_entry;
    GHashTable *labels;
};

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

static const gchar *menu_label(SettingMenuState *state, const gchar *key)
{
    const gchar *value = static_cast<const gchar *>(g_hash_table_lookup(state->labels, key));
    return value != NULL ? value : key;
}

static void shortcut_changed(GtkWidget *widget, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    gboolean enabled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state->shortcut_enabled));
    int modifier = gtk_combo_box_get_active(GTK_COMBO_BOX(state->shortcut_modifier));
    gtk_widget_set_sensitive(state->shortcut_modifier, enabled);
    aim_app_message_send(MSG_UI_SET_SHORTCUT, enabled ? modifier + 1 : 0);
}

static void show_shortcut_settings(GtkMenuItem *item, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    if (state->shortcut_window != NULL && gtk_widget_get_visible(state->shortcut_window)) {
        gtk_widget_hide(state->shortcut_window);
        return;
    }
    if (state->shortcut_window == NULL) {
        GtkBuilder *builder = gtk_builder_new_from_resource(
            "/org/gtk/aimapp/gui/gtk/shortcut-settings.ui");
        state->shortcut_window = GTK_WIDGET(gtk_builder_get_object(builder, "shortcut_window"));
        state->shortcut_enabled = GTK_WIDGET(gtk_builder_get_object(builder, "shortcut_enabled"));
        state->shortcut_modifier = GTK_WIDGET(gtk_builder_get_object(builder, "shortcut_modifier"));
        GtkWidget *note = GTK_WIDGET(gtk_builder_get_object(builder, "shortcut_note"));
        gtk_window_set_application(GTK_WINDOW(state->shortcut_window), GTK_APPLICATION(state->app));
        gtk_window_set_title(GTK_WINDOW(state->shortcut_window), menu_label(state, "Shortcut settings"));
        gtk_button_set_label(GTK_BUTTON(state->shortcut_enabled), menu_label(state, "Enable input mode shortcut"));
        gtk_label_set_text(GTK_LABEL(note), menu_label(state, "Shortcut note"));
        g_signal_connect(state->shortcut_window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
        g_object_unref(builder);
        AimAppClass *klass = AIM_APP_GET_CLASS(state->app);
        GVariant *reply = klass->im_proxy == NULL ? NULL : g_dbus_proxy_call_sync(
            klass->im_proxy, "GuiMessage", g_variant_new("(ii)", MSG_UI_GET_SHORTCUT, 0), G_DBUS_CALL_FLAGS_NONE, 2000, NULL, NULL);
        int shortcut = 2;
        if (reply != NULL) {
            g_variant_get(reply, "(i)", &shortcut);
            g_variant_unref(reply);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(state->shortcut_modifier), shortcut > 0 ? shortcut - 1 : 1);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->shortcut_enabled), shortcut > 0);
        gtk_widget_set_sensitive(state->shortcut_modifier, shortcut > 0);
        g_signal_connect(state->shortcut_enabled, "toggled", G_CALLBACK(shortcut_changed), state);
        g_signal_connect(state->shortcut_modifier, "changed", G_CALLBACK(shortcut_changed), state);
    }
    gtk_widget_show_all(state->shortcut_window);
    gtk_window_present(GTK_WINDOW(state->shortcut_window));
}

static void copy_input(GtkWidget *widget, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(state->input_entry));
    if (*text == '\0')
        return;
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, text, -1);
    gtk_clipboard_store(clipboard);
    gtk_entry_set_text(GTK_ENTRY(state->input_entry), "");
}

static gboolean input_key_pressed(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if (event->keyval != GDK_KEY_Escape)
        return FALSE;
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    gtk_widget_hide(state->input_window);
    return TRUE;
}

static void position_menu_window(GtkWindow *window, int &position_x, int &position_y)
{
    GdkDisplay *display = gtk_widget_get_display(GTK_WIDGET(window));
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);
    if (monitor == NULL)
        monitor = gdk_display_get_monitor_at_window(display, gtk_widget_get_window(GTK_WIDGET(window)));
    gtk_window_get_position(window, &position_x, &position_y);
    if (monitor == NULL)
        return;

    int width, height;
    gtk_window_get_size(window, &width, &height);
    GdkRectangle area;
    gdk_monitor_get_workarea(monitor, &area);
    position_x = MAX(area.x, area.x + area.width - width - 12);
    position_y = MAX(area.y, area.y + area.height - height - 12);
    gtk_window_move(window, position_x, position_y);
}

static void show_input_window(GtkMenuItem *item, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    if (state->input_window != NULL && gtk_widget_get_visible(state->input_window)) {
        gtk_widget_hide(state->input_window);
        return;
    }
    if (state->input_window == NULL) {
        GtkBuilder *builder = gtk_builder_new_from_resource(
            "/org/gtk/aimapp/gui/gtk/input-copy.ui");
        state->input_window = GTK_WIDGET(gtk_builder_get_object(builder, "input_window"));
        state->input_entry = GTK_WIDGET(gtk_builder_get_object(builder, "input_entry"));
        GtkWidget *copy = GTK_WIDGET(gtk_builder_get_object(builder, "copy_button"));
        gtk_window_set_application(GTK_WINDOW(state->input_window), GTK_APPLICATION(state->app));
        gtk_window_set_title(GTK_WINDOW(state->input_window), menu_label(state, "Input and copy"));
        gtk_window_set_keep_above(GTK_WINDOW(state->input_window), TRUE);
        g_object_set(state->input_entry, "im-module", IM_ID, NULL);
        gtk_entry_set_placeholder_text(GTK_ENTRY(state->input_entry), menu_label(state, "Input placeholder"));
        gtk_button_set_label(GTK_BUTTON(copy), menu_label(state, "Copy"));
        g_signal_connect(state->input_window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
        g_signal_connect(state->input_window, "key-press-event", G_CALLBACK(input_key_pressed), state);
        g_object_unref(builder);
        g_signal_connect(copy, "clicked", G_CALLBACK(copy_input), state);
        g_signal_connect(state->input_entry, "activate", G_CALLBACK(copy_input), state);
    }
    gtk_widget_show_all(state->input_window);
    int position_x, position_y;
    position_menu_window(GTK_WINDOW(state->input_window), position_x, position_y);
    gtk_window_present(GTK_WINDOW(state->input_window));
    gtk_widget_grab_focus(state->input_entry);
}

static void toggle_im_window(GtkCheckMenuItem *item, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    AimAppClass *klass = AIM_APP_GET_CLASS(state->app);
    klass->bshow_imwin = gtk_check_menu_item_get_active(item);
    if (!klass->bshow_imwin) {
        gtk_widget_hide(GTK_WIDGET(klass->imwin));
        return;
    }
    gtk_widget_show_all(GTK_WIDGET(klass->imwin));
    position_menu_window(GTK_WINDOW(klass->imwin), klass->x, klass->y);
    aim_win_set_pos(klass->imwin, klass->x, klass->y);
}

static void destroy_menu_state(gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    g_hash_table_unref(state->labels);
    if (state->shortcut_window != NULL)
        gtk_widget_destroy(state->shortcut_window);
    if (state->input_window != NULL)
        gtk_widget_destroy(state->input_window);
    g_free(state);
}

GtkMenu *setting_menu_new(GApplication *app)
{
    const gchar *labels[] = {"Input and copy", "Shortcut settings"};
    GCallback callbacks[] = {G_CALLBACK(show_input_window),
        G_CALLBACK(show_shortcut_settings)};

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
    GtkWidget *im_window = gtk_check_menu_item_new_with_label(menu_label(state, "Input method window"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), im_window);
    g_signal_connect(im_window, "toggled", G_CALLBACK(toggle_im_window), state);

    for (guint index = 0; index < G_N_ELEMENTS(labels); ++index) {
        GtkWidget *item = gtk_menu_item_new_with_label(menu_label(state, labels[index]));
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
        g_signal_connect(item, "activate", callbacks[index], state);
    }
    gtk_widget_show_all(menu);
    return GTK_MENU(menu);
}
