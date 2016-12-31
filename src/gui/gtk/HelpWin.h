#ifndef _HELPWIN_H_
#define _HELPWIN_H_

#include <gtk/gtk.h>


typedef struct _HelpWin         HelpWin;
typedef struct _HelpWinClass    HelpWinClass;

#define HELP_WIN_TYPE             (help_win_get_type ())
#define HELP_WIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj),HELP_WIN_TYPE, HelpWin))
#define HELP_WIN_CLASS(klass)     (G_TYPE_CHECK_INSTANCE_CAST ((klass), HELP_WIN_TYPE, HelpWinClass))
#define HELP_WIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS  ((obj), HELP_WIN_TYPE,   HelpWinClass))

struct _HelpWin
{
    GtkWindow  parent;
};

struct _HelpWinClass
{
    GtkWindowClass parent_class;
    //GtkWidget *text_view;

    int x,y;
};

extern GType     help_win_get_type (void);
extern void      help_win_show_hide(HelpWin *win);
extern void      help_win_hide(HelpWin *win);
extern HelpWin*  help_win_new      (int x, int y);

#endif
