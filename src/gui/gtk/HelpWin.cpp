#include <gtk/gtk.h>

#include "HelpWin.h"

G_DEFINE_TYPE(HelpWin, help_win, GTK_TYPE_WINDOW);

#define HP_WIN_H 520
#define HP_WIN_W 560

static GtkWidget *section(const gchar *title, const gchar *body)
{
    GtkWidget *frame = gtk_frame_new(NULL);
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    GtkWidget *heading = gtk_label_new(NULL);
    GtkWidget *content = gtk_label_new(body);

    gchar *markup = g_markup_printf_escaped("<b>%s</b>", title);
    gtk_label_set_markup(GTK_LABEL(heading), markup);
    g_free(markup);
    gtk_label_set_xalign(GTK_LABEL(heading), 0.0);
    gtk_label_set_xalign(GTK_LABEL(content), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(content), TRUE);
    gtk_label_set_selectable(GTK_LABEL(content), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(content), 64);
    gtk_widget_set_margin_start(box, 14);
    gtk_widget_set_margin_end(box, 14);
    gtk_widget_set_margin_top(box, 12);
    gtk_widget_set_margin_bottom(box, 12);
    gtk_box_pack_start(GTK_BOX(box), heading, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), content, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(frame), box);
    return frame;
}

static void help_win_init(HelpWin *win)
{
}

static void help_win_class_init(HelpWinClass *klass)
{
}

void help_win_hide(HelpWin *win)
{
    gtk_widget_hide(GTK_WIDGET(win));
}

void help_win_show_hide(HelpWin *win)
{
    if (gtk_widget_get_visible(GTK_WIDGET(win))) {
        gtk_widget_hide(GTK_WIDGET(win));
        return;
    }

    gtk_widget_show_all(GTK_WIDGET(win));
    gtk_window_present(GTK_WINDOW(win));
}

HelpWin *help_win_new(int x, int y)
{
    HelpWin *helpwin = HELP_WIN(g_object_new(HELP_WIN_TYPE,
        "type", GTK_WINDOW_TOPLEVEL,
        "title", "关于阿尔法输入法",
        "resizable", TRUE,
        "default-width", HP_WIN_W,
        "default-height", HP_WIN_H,
        "window-position", GTK_WIN_POS_CENTER,
        NULL));
    HelpWinClass *klass = HELP_WIN_GET_CLASS(helpwin);
    GtkWidget *titlebar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(titlebar), "帮助");
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(titlebar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(helpwin), titlebar);
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *title = gtk_label_new(NULL);
    GtkWidget *subtitle = gtk_label_new("简洁、离线、低资源消耗的 Linux 输入法");
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);

    gtk_container_add(GTK_CONTAINER(helpwin), root);
    gtk_widget_set_margin_start(header, 22);
    gtk_widget_set_margin_end(header, 22);
    gtk_widget_set_margin_top(header, 18);
    gtk_widget_set_margin_bottom(header, 14);
    gtk_label_set_markup(GTK_LABEL(title), "<big><b>阿尔法输入法</b></big>");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0);
    gtk_label_set_xalign(GTK_LABEL(subtitle), 0.0);
    gtk_style_context_add_class(gtk_widget_get_style_context(subtitle), "dim-label");
    gtk_box_pack_start(GTK_BOX(header), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header), subtitle, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(root), header, FALSE, FALSE, 0);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_margin_start(content, 18);
    gtk_widget_set_margin_end(content, 18);
    gtk_widget_set_margin_top(content, 4);
    gtk_widget_set_margin_bottom(content, 18);
    gtk_box_pack_start(GTK_BOX(content), section("功能特性",
        "自动学习生成用户词组，只学习汉字；根据输入频度排序；支持模糊拼音，只需输入词组部分拼音字母。"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), section("快捷键",
        "输入法切换：Ctrl + Space、Ctrl + Shift\n"
        "中英文切换：左 Shift\n"
        "符号切换：右 Shift\n"
        "提交：Space 或 Enter\n"
        "取消：Esc\n"
        "向前翻页：Page Up、左方向键、上方向键、-\n"
        "向后翻页：Page Down、右方向键、下方向键、="), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), section("软件兼容性",
        "支持 XIM、GTK 和 Qt 输入协议。使用私有 Qt/Gtk 库的应用可能需要单独处理。\n"
        "Linux 微信使用静态编译的私有 Qt，暂不支持，可使用托盘菜单的“输入并复制”，再粘贴到应用中。"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), section("WPS",
        "WPS 使用私有 Qt。需要下载 Qt 官方 SDK 后编译，详见项目中的 im-private/wps/Qt5.12/README。"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(content), section("关于",
        "版本：1.5\n"
        "作者：李力琼 (joni lee)\n"
        "联系：joni_lee@outlook.com\n"
        "许可证：GNU GPLv3。"), FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(scroll), content);
    gtk_box_pack_start(GTK_BOX(root), scroll, TRUE, TRUE, 0);

    g_signal_connect(helpwin, "delete-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    klass->x = x;
    klass->y = y;
    return helpwin;
}
