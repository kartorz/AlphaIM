#include <gtk/gtk.h>

#include "AimApp.h"
#include "SettingMenu.h"
#include "aim.h"

static void setting_written(GObject *source, GAsyncResult *result, gpointer user_data)
{
    GError *error = NULL;
    GVariant *reply = g_dbus_proxy_call_finish(G_DBUS_PROXY(source), result, &error);
    if (reply != NULL)
        g_variant_unref(reply);
    if (error != NULL) {
        g_warning("WriteInt failed: %s", error->message);
        g_error_free(error);
    }
}

static void setting_write_int(SettingMenuState *state, const gchar *key, int value)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(state->app);
    GVariant *parameters = g_variant_ref_sink(g_variant_new("(si)", key, value));
    if (klass->im_proxy != NULL)
        g_dbus_proxy_call(klass->im_proxy, "WriteInt", parameters,
            G_DBUS_CALL_FLAGS_NONE, 2000, NULL, setting_written, NULL);
    g_variant_unref(parameters);
}

static void read_settings(SettingMenuState *state, int *shortcut, int *language,
    int *mode, gboolean *learn)
{
    AimAppClass *klass = AIM_APP_GET_CLASS(state->app);
    if (klass->im_proxy == NULL)
        return;
    GError *error = NULL;
    GVariant *reply = g_dbus_proxy_call_sync(klass->im_proxy, "ReadSettings", NULL,
        G_DBUS_CALL_FLAGS_NONE, 2000, NULL, &error);
    if (reply == NULL) {
        g_warning("ReadSettings failed: %s", error->message);
        g_clear_error(&error);
        return;
    }
    GVariant *settings = g_variant_get_child_value(reply, 0);
    g_variant_lookup(settings, "onOffKey", "i", shortcut);
    g_variant_lookup(settings, "imSwitchKey", "i", language);
    g_variant_lookup(settings, "icMode", "i", mode);
    g_variant_lookup(settings, "learnEn", "b", learn);
    g_variant_unref(settings);
    g_variant_unref(reply);
}

static void onoffkey_changed(GtkWidget *widget, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    if (state->loading)
        return;
    setting_write_int(state, "onOffKey",
        gtk_combo_box_get_active(GTK_COMBO_BOX(state->onoffkey_modifier)));
}

static void language_changed(GtkWidget *widget, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    if (state->loading)
        return;
    setting_write_int(state, "imSwitchKey",
        gtk_combo_box_get_active(GTK_COMBO_BOX(state->imswitchkey_modifier)) + 1);
}

static void ic_mode_changed(GtkWidget *widget, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    if (state->loading || !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
        return;
    setting_write_int(state, "icMode", widget == state->ic_mode_global ? 1 : 0);
}

static void learning_changed(GtkWidget *widget, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    if (!state->loading)
        setting_write_int(state, "learnEn",
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(state->learn_enabled)) ? 1 : 0);
}

static void connect_setting_signal(GtkBuilder *builder, GObject *object,
    const gchar *signal_name, const gchar *handler_name, GObject *connect_object,
    GConnectFlags flags, gpointer user_data)
{
    GCallback callback = NULL;
    if (g_str_equal(handler_name, "onoffkey_changed"))
        callback = G_CALLBACK(onoffkey_changed);
    else if (g_str_equal(handler_name, "language_changed"))
        callback = G_CALLBACK(language_changed);
    else if (g_str_equal(handler_name, "ic_mode_changed"))
        callback = G_CALLBACK(ic_mode_changed);
    else if (g_str_equal(handler_name, "learning_changed"))
        callback = G_CALLBACK(learning_changed);
    if (callback != NULL)
        g_signal_connect_data(object, signal_name, callback, user_data, NULL, flags);
}

void setting_window_show(GtkMenuItem *item, gpointer user_data)
{
    SettingMenuState *state = static_cast<SettingMenuState *>(user_data);
    if (state->settings_window == NULL) {
        GtkBuilder *builder = gtk_builder_new_from_resource(
            "/org/gtk/aimapp/gui/gtk/shortcut-settings.ui");
        state->settings_window = GTK_WIDGET(gtk_builder_get_object(builder, "settings_window"));
        state->onoffkey_modifier = GTK_WIDGET(gtk_builder_get_object(builder, "onoffkey_modifier"));
        state->imswitchkey_modifier = GTK_WIDGET(gtk_builder_get_object(builder, "imswitchkey_modifier"));
        state->ic_mode_per_app = GTK_WIDGET(gtk_builder_get_object(builder, "ic_mode_per_app"));
        state->ic_mode_global = GTK_WIDGET(gtk_builder_get_object(builder, "ic_mode_global"));
        state->learn_enabled = GTK_WIDGET(gtk_builder_get_object(builder, "learn_enabled"));
        gtk_window_set_application(GTK_WINDOW(state->settings_window), GTK_APPLICATION(state->app));
        gtk_window_set_title(GTK_WINDOW(state->settings_window), setting_menu_label(state, "Input method settings"));
        GtkHeaderBar *header = GTK_HEADER_BAR(gtk_builder_get_object(builder, "settings_header"));
        gtk_header_bar_set_title(header, setting_menu_label(state, "Input method settings"));
        gtk_header_bar_set_subtitle(header, setting_menu_label(state, "Settings subtitle"));
        gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "shortcut_label")),
            setting_menu_label(state, "Switch input method"));
        gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "language_label")),
            setting_menu_label(state, "Switch language"));
        gtk_button_set_label(GTK_BUTTON(state->ic_mode_per_app), setting_menu_label(state, "Per application"));
        gtk_button_set_label(GTK_BUTTON(state->ic_mode_global), setting_menu_label(state, "Global"));
        gtk_button_set_label(GTK_BUTTON(state->learn_enabled), setting_menu_label(state, "Learning"));
        const gchar *label_ids[] = {"shortcut_section_title", "ic_mode_section_title", "features_section_title"};
        const gchar *label_keys[] = {"Shortcut key", "Input context", "Features"};
        for (guint index = 0; index < G_N_ELEMENTS(label_ids); ++index)
            gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, label_ids[index])),
                setting_menu_label(state, label_keys[index]));
        g_signal_connect(state->settings_window, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
        gtk_builder_connect_signals_full(builder, connect_setting_signal, state);
        g_object_unref(builder);
    }
    state->loading = TRUE;
    int shortcut = 1, language = 0, mode = 0;
    gboolean learn = TRUE;
    read_settings(state, &shortcut, &language, &mode, &learn);
    gtk_combo_box_set_active(GTK_COMBO_BOX(state->onoffkey_modifier), shortcut);
    gtk_combo_box_set_active(GTK_COMBO_BOX(state->imswitchkey_modifier), language > 0 ? language - 1 : 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mode == 0 ? state->ic_mode_per_app : state->ic_mode_global), TRUE);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(state->learn_enabled), learn);
    state->loading = FALSE;
    gtk_widget_show_all(state->settings_window);
    gtk_window_present(GTK_WINDOW(state->settings_window));
}
