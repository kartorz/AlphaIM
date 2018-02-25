#ifndef _SETTINGWIN_H_
#define _SETTINGWIN_H_

#include <gtk/gtk.h>


typedef struct _SettingWin         SettingWin;
typedef struct _SettingWinClass    SettingWinClass;

#define SETTING_WIN_TYPE             (setting_win_get_type ())
#define SETTING_WIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj),SETTING_WIN_TYPE, SettingWin))
#define SETTING_WIN_CLASS(klass)     (G_TYPE_CHECK_INSTANCE_CAST ((klass), SETTING_WIN_TYPE, SettingWinClass))
#define SETTING_WIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS  ((obj), SETTING_WIN_TYPE,   SettingWinClass))

struct _SettingWin
{
    GtkWindow  parent;
};

struct _SettingWinClass
{
    GtkWindowClass parent_class;
    GtkWidget *menu;

    int x,y;
};

extern void      setting_win_show_hide(SettingWin *win);
extern void      setting_win_hide(SettingWin *win);
extern GType     setting_win_get_type (void);
extern SettingWin*  setting_win_new      (GtkApplication *gtkapp, int x, int y);

#endif
