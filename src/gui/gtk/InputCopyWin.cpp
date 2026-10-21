#include <gtk/gtk.h>

#include "SettingMenu.h"
#include "aim.h"

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

void input_copy_window_show(GtkMenuItem *item, gpointer user_data)
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
        gtk_window_set_title(GTK_WINDOW(state->input_window), setting_menu_label(state, "Input and copy"));
        gtk_window_set_keep_above(GTK_WINDOW(state->input_window), TRUE);
        g_object_set(state->input_entry, "im-module", IM_ID, NULL);
        gtk_entry_set_placeholder_text(GTK_ENTRY(state->input_entry), setting_menu_label(state, "Input placeholder"));
        gtk_button_set_label(GTK_BUTTON(copy), setting_menu_label(state, "Copy"));
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
