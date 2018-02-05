#ifndef _GAPPLICATION_H_
#define _GAPPLICATION_H_

#include <gtk/gtk.h>
#include "AimWin.h"
#include "IcWin.h"
#include "HelpWin.h"

#include "aim.h"
#include "GuiMessager.h"

typedef struct _AimApp       AimApp;
typedef struct _AimAppClass  AimAppClass;

#define AIM_APP_TYPE             (aim_app_get_type ())
#define AIM_APP(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_APP_TYPE, AimApp))
#define AIM_APP_CLASS(klass)     (G_TYPE_CHECK_INSTANCE_CAST ((klass), AIM_APP_TYPE, AimAppClass))
#define AIM_APP_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_APP_TYPE, AimAppClass))

struct _AimApp
{
    GtkApplication parent;
};

struct _AimAppClass
{
    GtkApplicationClass parent_class;

    GuiMessager         *gui_messager;
    IcWin               *icwin;
    AimWin              *imwin;
    HelpWin             *hpwin;

    GtkStatusIcon       *systray;
    GtkWidget           *systray_img_en;
    GtkWidget           *systray_img_cn;
    GtkWidget           *systray_img_app;
    bool                bshow_imwin;
    int                 x,y;
	GDBusProxy          *im_proxy;
	GDBusProxy          *event_proxy;
};

#ifdef __cplusplus
extern "C" {
#endif

extern GType   aim_app_get_type       (void);
extern AimApp* aim_app_new            (void);
extern int     aim_app_main           (int argc, char* argv[]);

#ifdef __cplusplus
}
#endif

#endif
