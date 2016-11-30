#ifndef _AIMWIN_H_
#define _AIMWIN_H_

#include <gtk/gtk.h>
#include "HelpWin.h"

typedef struct _AimWin         AimWin;
typedef struct _AimWinClass    AimWinClass;

#define AIM_WIN_TYPE             (aim_win_get_type ())
#define AIM_WIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_WIN_TYPE, AimWin))
#define AIM_WIN_CLASS(klass)     (G_TYPE_CHECK_INSTANCE_CAST ((klass), AIM_WIN_TYPE, AimWinClass))
#define AIM_WIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_WIN_TYPE,    AimWinClass))

struct _AimWin
{
    GtkWindow  parent;
};

struct _AimWinClass
{
    GtkApplicationWindowClass parent_class;

    GtkWidget *lan_button;
    GtkWidget *pun_button;

    int  x;
    int  y;
};

extern GType   aim_win_get_type    (void);
extern AimWin* aim_win_new         (int x, int y);
extern void    aim_win_enable_im   (AimWin *win, bool en);
extern void    aim_win_show_hide   (AimWin *win);
extern void    aim_win_set_pos     (AimWin *win, int x, int y);
extern void    aim_win_switch_lan(AimWin *win, bool is_cn);
extern void    aim_win_switch_pun(AimWin *win, bool is_cn);
#endif
