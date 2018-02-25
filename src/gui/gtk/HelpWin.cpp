#include <gtk/gtk.h>
#include <string>

#include "HelpWin.h"

G_DEFINE_TYPE(HelpWin, help_win, GTK_TYPE_WINDOW /*GTK_TYPE_APPLICATION_WINDOW*/);

#define HP_WIN_H  300
#define HP_WIN_W  240

static void help_win_init(HelpWin *win)
{
}

static void help_win_class_init(HelpWinClass *klass)
{

}

void help_win_hide(HelpWin *win)
{
    HelpWinClass *klass = HELP_WIN_GET_CLASS(win);
    gtk_widget_hide(GTK_WIDGET (win));
}

void help_win_show_hide(HelpWin *win)
{
    HelpWinClass *klass = HELP_WIN_GET_CLASS(win);

    if (gtk_widget_get_mapped(GTK_WIDGET (win))) {
        gtk_widget_hide(GTK_WIDGET (win));
    } else {
        gtk_widget_show_all(GTK_WIDGET (win));

        GdkWindow *gdk_win = gtk_widget_get_window(GTK_WIDGET (win));
        gdk_window_move(gdk_win,  klass->x - 50,  klass->y - HP_WIN_H);
    }
}


HelpWin *help_win_new(int x, int y)
{
    HelpWin* helpwin = (HelpWin *)g_object_new (HELP_WIN_TYPE,
                                          "type", GTK_WINDOW_POPUP,
                                          "decorated", FALSE,
                                          "resizable", FALSE,
                                          "window-position", GTK_WIN_POS_CENTER,
                                          "accept-focus", FALSE,
                                          NULL);
    HelpWinClass *klass = HELP_WIN_GET_CLASS(helpwin);
    GtkWidget *win = ( GtkWidget *)(GTK_WINDOW(helpwin));
    gtk_widget_set_size_request(win, HP_WIN_W, HP_WIN_H);
    GtkWidget *scrollwin = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *input_label = gtk_label_new(NULL);
    std::string markup = "<span foreground=\"blue\" size=\"x-large\">";
    markup += "版本号: 1.0\n\n";
    markup += "快捷键\n";
    markup += "\n";
    markup += "  - 打开关闭输入法\n";
    markup += "      . Ctr  + SPACE\n";
    markup += "      . Shift + SPACE\n";
    markup += "\n";
    markup += "  - 中英文切换\n";
    markup += "      . 左(Ctr + Shift)";
    markup += "\n";
    markup += "  - 中英文符号切换\n";
	markup += "      . 右(Ctr + Shift)";
    markup += "\n";
    markup += "  - 提交\n";
    markup += "      . SPACE\n";
    markup += "      . Enter\n";
    markup += "\n";
    markup += "  - 取消\n";
    markup += "      . Esc\n";
    markup += "      . Ctr + C\n";
    markup += "\n";
    markup += "  - 向前翻页\n";
    markup += "      . Ctr + F\n";
    markup += "      . PgUp\n";
    markup += "      . 左方向键\n";
	markup += "      . 上方向键\n";
    markup += "      . '-'键\n";
    markup += "\n";
    markup += "  - 向后翻页\n";
    markup += "      . Ctr + J\n";
    markup += "      . PgDn\n";
    markup += "      . 右方向键\n";
	markup += "      . 下方向键\n";
    markup += "      . '=' 键\n";
    markup += "</span>";
    gtk_label_set_markup (GTK_LABEL (input_label), markup.c_str());

#if 0
    GtkWidget *text_view = gtk_text_view_new();
    //gtk_text_view_set_justification((GtkTextView *)text_view, GTK_JUSTIFY_CENTER);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
    gtk_text_buffer_set_text (buffer, "快捷键\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "  - 打开关闭输入法\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . Ctr  + SPACE\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . Shift + SPACE\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "  - 中英文切换\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . 左Shift\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "  - 中英文符号切换\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . 右Shift\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "  - 提交\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . SPACE\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . Enter\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "  - 取消\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . Esc\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . Ctr + C\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "  - 向前翻页\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . Ctr + F\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . PgUp\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . 左方向键盘\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . '-'键\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "  - 向后翻页\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . Ctr + J\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . PgDn\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . 右方向键盘\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, "      . '=' 键\n", -1);
    gtk_container_add(GTK_CONTAINER(scrollwin), text_view);
#endif
    gtk_container_add(GTK_CONTAINER(scrollwin), input_label);
    gtk_container_add (GTK_CONTAINER (helpwin), scrollwin);
    //gtk_container_add (GTK_CONTAINER (helpwin), input_label);

    /* Must after 'gtk_container_add'.
     * If not call 'gtk_widget_show_all' this time, call this function instead of. 
     * Or, the app will be blocked when call 'gtk_widget_show_all' in the future.
     */
    //gtk_widget_realize(GTK_WIDGET (text_view));
    //gtk_widget_show_all(GTK_WIDGET (win));
    //gtk_widget_show(GTK_WIDGET (text_view));
    //gtk_widget_show_all(GTK_WIDGET (win));
 
    //gtk_widget_hide (GTK_WIDGET (win));
    //klass->text_view = NULL;

    klass->x = x;
    klass->y = y;
    return helpwin;
}
