#ifndef _GUIMESSAGER_H_
#define _GUIMESSAGER_H_

#include <gtk/gtk.h>
#include "MessageQueue.h"


typedef struct _GuiMessager  GuiMessager;
typedef struct _GuiMessagerClass  GuiMessagerClass;

#define GUI_MESSAGER_TYPE             (gui_messager_get_type ())
#define GUI_MESSAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GUI_MESSAGER_TYPE, GuiMessager))
#define GUI_MESSAGER_CLASS(klass)     (G_TYPE_CHECK_INSTANCE_CAST ((klass), GUI_MESSAGER_TYPE, GuiMessagerClass))
#define GUI_MESSAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GUI_MESSAGER_TYPE, GuiMessagerClass))

struct _GuiMessager
{
    GObject  parent;
};

struct _GuiMessagerClass
{
    GObjectClass parent_class;

    GThread         *thd;
    bool            req_abort;
    MessageQueue    *msg_queue;
    guint           show_icwin_id;
    guint           active_im_id;
    guint           disactive_im_id;
};

#ifdef __cplusplus
extern "C" {
#endif

extern GuiMessager*    gui_messger_new    (MessageQueue *q);
extern void            gui_messager_abort (GuiMessager *msgr);

#ifdef __cplusplus
}
#endif

#endif
