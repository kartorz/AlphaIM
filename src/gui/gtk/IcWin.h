#ifndef __ICWIN_H
#define __ICWIN_H

#include <gtk/gtk.h>
#include <string>
#include <vector>

#include "iIM.h"

typedef struct _IcWin         IcWin;
typedef struct _IcWinClass    IcWinClass;

#define IC_WIN_TYPE             (ic_win_get_type ())
#define IC_WIN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj),IC_WIN_TYPE, IcWin))
#define IC_WIN_CLASS(klass)     (G_TYPE_CHECK_INSTANCE_CAST ((klass), IC_WIN_TYPE, IcWinClass))
#define IC_WIN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS  ((obj), IC_WIN_TYPE,   IcWinClass))

#define PREEDIT_ITEMS_MAX    IM_ITEM_PAGE_SIZE

struct _IcWin
{
    GtkWindow  parent;
};

struct _IcWinClass
{
    GtkWindowClass parent_class;
    
    GtkWidget *input_label;
    GtkWidget *preedit_label[PREEDIT_ITEMS_MAX];
};

//extern void    ic_win_showinfo (void);
extern GType   ic_win_get_type (void);
extern IcWin*  ic_win_new      (void);

extern void ic_win_refresh(IcWin *win,
                    gint32 x,
                    gint32 y,
                    gint32 w,
                    gint32 h,
					gchar *input,
					gchar *items);
#endif
