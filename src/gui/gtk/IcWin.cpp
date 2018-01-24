#include <gtk/gtk.h>
#include <string>

#include <stdio.h>

#include "IcWin.h"

G_DEFINE_TYPE(IcWin, ic_win, GTK_TYPE_WINDOW /*GTK_TYPE_APPLICATION_WINDOW*/);

static void ic_win_init(IcWin *win)
{
}

static void ic_win_class_init(IcWinClass *klass)
{

}

void ic_win_refresh(IcWin *win,
                    gint32 x,
                    gint32 y,
                    gint32 w,
                    gint32 h,
					gchar *input,
					gchar *items)
{
    GtkWindow *icwin = GTK_WINDOW (win);
    IcWinClass *klass = IC_WIN_GET_CLASS(win);

    std::string markup = "<span foreground=\"blue\" size=\"x-large\">";
    markup += "  " + std::string(input) + "</span>";
    gtk_label_set_markup (GTK_LABEL (klass->input_label), markup.c_str());
    //gtk_label_set_text(GTK_LABEL (klass->input_label), strInput.c_str());
    int pi = 0;
	gchar ** itemArray = g_strsplit (items, " ", 0);
	if (itemArray) {
		for(gchar ** item = itemArray; *item; ++item) {
			if (g_strcmp0(*item, "") == 0)
				continue;
			std::string text = "  ";
			char pichr = 0x31 + pi;
			text += pichr;
			text += ": ";
			text += *item;
			//gtk_label_set_text(GTK_LABEL (klass->preedit_label[pi]), text.c_str());
			if (pi == 0)
				markup = "<span foreground=\"blue\" size=\"x-large\" background=\"green\">";
			else
				markup = "<span foreground=\"blue\" size=\"x-large\">";
			markup += text +  "</span>";
			gtk_label_set_markup (GTK_LABEL (klass->preedit_label[pi]), markup.c_str());
			if (++pi >= PREEDIT_ITEMS_MAX)
				break;
		}

		g_strfreev(itemArray);
	}

    for (; pi < PREEDIT_ITEMS_MAX; pi++)
        gtk_label_set_text(GTK_LABEL (klass->preedit_label[pi]), " ");

    gtk_widget_show_all(GTK_WIDGET (icwin));

    gtk_window_resize(icwin, w, h);
    gtk_window_move(icwin, x, y);

}

IcWin *ic_win_new()
{
    IcWin* icwin = (IcWin *)g_object_new (IC_WIN_TYPE,
                                          "type", GTK_WINDOW_POPUP,
                                          "decorated", FALSE,
                                          "resizable", FALSE,
                                          "window-position", GTK_WIN_POS_CENTER,
                                          "accept-focus", FALSE,
                                          NULL);
    IcWinClass *klass = IC_WIN_GET_CLASS(icwin);

    GtkWidget *window = ( GtkWidget *)(GTK_WINDOW(icwin));
    //gtk_widget_set_size_request(window, ICWIN_W, ICWIN_H);    
   // GtkWidget *gridc = gtk_grid_new();
    GtkWidget *gridc = (GtkWidget *) g_object_new(GTK_TYPE_GRID,
                                                  "row-spacing", 10,
                                                  NULL);
    gtk_container_add(GTK_CONTAINER (window), gridc);
    //g_signal_connect (button, "clicked", G_CALLBACK (print_hello), NULL);

    klass->input_label = gtk_label_new(NULL);
    //gtk_label_set_justify ((GtkLabel *)klass->input_label, GTK_JUSTIFY_LEFT);
    gtk_widget_set_halign (klass->input_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (gridc), klass->input_label, 0, 0, PREEDIT_ITEMS_MAX, 1);

    for (int i = 0; i < PREEDIT_ITEMS_MAX; i++) {
        klass->preedit_label[i] = gtk_label_new(NULL);
        //gtk_label_set_markup (GTK_LABEL (klass->preedit_label[i]), "<span foreground=\"blue\" size=\"x-large\"");
        gtk_grid_attach(GTK_GRID (gridc), klass->preedit_label[i], i, 1, 1, 1);
    }

    /*GtkWidget *button = gtk_button_new_with_label("<");
    gtk_grid_attach(GTK_GRID (gridc), button, PREEDIT_ITEMS_MAX, 1, 1, 1);

    button = gtk_button_new_with_label(">");
    gtk_grid_attach(GTK_GRID (gridc), button, PREEDIT_ITEMS_MAX+1, 1, 1, 1);*/

    //gtk_widget_show_all (window);
    return icwin;
}
