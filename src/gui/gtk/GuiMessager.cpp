#include "GuiMessager.h"

G_DEFINE_TYPE(GuiMessager, gui_messager, G_TYPE_OBJECT);

static gpointer gui_messager_work(gpointer data);

extern gboolean aim_app_on_show_icwin(gpointer user_data);
extern gboolean aim_app_on_active_im(gpointer user_data);
extern gboolean aim_app_on_disactive_im(gpointer user_data);
extern gboolean aim_app_on_hide_icwin(gpointer user_data);
extern gboolean aim_app_on_switch_lan(gpointer user_data);
extern gboolean aim_app_on_switch_pun(gpointer user_data);

static void gui_messager_init(GuiMessager *msgr)
{
    GuiMessagerClass* kclass = GUI_MESSAGER_GET_CLASS(msgr);
    kclass->thd = g_thread_new("guimessager", gui_messager_work, (gpointer)msgr);
}

static void gui_messager_dispose(GObject *gobject)
{
    GuiMessagerClass* klass = GUI_MESSAGER_GET_CLASS(gobject);
    g_thread_join(klass->thd);
    g_thread_unref(klass->thd);
}

static void gui_messager_finalize(GObject *gobject)
{
}

static void gui_messager_class_init(GuiMessagerClass *klass)
{
    G_OBJECT_CLASS (klass)->dispose   = gui_messager_dispose;
    G_OBJECT_CLASS (klass)->finalize = gui_messager_finalize;

    klass->thd = NULL;
    klass->req_abort = false;
}

static gpointer gui_messager_work(gpointer data)
{
    GuiMessagerClass* klass = GUI_MESSAGER_GET_CLASS((GuiMessager *)data);
    do{
        Message msg;
        if (klass->msg_queue->pop(msg)) {
            switch (msg.id) {
            case MSG_IM_INPUT: {
                Message *pmsg = new Message(msg);
                gdk_threads_add_idle(aim_app_on_show_icwin, (gpointer)pmsg);

                break;
            }

            case MSG_IM_ON: {
                //g_signal_emit(data, klass->active_im_id, 0);
                gdk_threads_add_idle(aim_app_on_active_im, NULL);

                break;
            }

            case MSG_IM_OFF: {
                gdk_threads_add_idle(aim_app_on_disactive_im, NULL);

                break;
            }

            case MSG_IM_CLOSE: {
                gdk_threads_add_idle(aim_app_on_hide_icwin, NULL);
            }

            case MSG_IM_COMMIT: {
                gdk_threads_add_idle(aim_app_on_hide_icwin, NULL);

                break;
            }

            case MSG_IM_CN: {
                gboolean *is_cn = new gboolean;
                *is_cn = true;
                gdk_threads_add_idle(aim_app_on_switch_lan, is_cn);

                break;
            }

            case MSG_IM_EN: {
                gboolean *is_cn = new gboolean;
                *is_cn = false;
                gdk_threads_add_idle(aim_app_on_switch_lan, is_cn);

                break;
            }

            case MSG_IM_CPUN: {
                gboolean *is_cn = new gboolean;
                *is_cn = true;
                gdk_threads_add_idle(aim_app_on_switch_pun, is_cn);

                break;
            }

            case MSG_IM_EPUN: {
                gboolean *is_cn = new gboolean;
                *is_cn = false;
                gdk_threads_add_idle(aim_app_on_switch_pun, is_cn);

                break;
            }
                    
            case MSG_QUIT: {
                klass->req_abort = true;

                break;
            }

            default:
                break;
            }

        } else {
            klass->req_abort = true;
            //printf("{GuiMessager} no message eixt\n");
            break;
        }
    } while(!klass->req_abort);
}

void gui_messager_abort(GuiMessager *msgr)
{
    GuiMessagerClass* klass = GUI_MESSAGER_GET_CLASS(msgr);
	if(klass->thd)
	{
	    klass->req_abort = true;
        klass->msg_queue->push(MSG_QUIT);
    }
}

GuiMessager *gui_messger_new(MessageQueue *q)
{
    GuiMessager *msgr = (GuiMessager*)g_object_new (GUI_MESSAGER_TYPE, NULL);
    GuiMessagerClass* klass = GUI_MESSAGER_GET_CLASS(msgr);
    klass->msg_queue = q;

    return msgr;
}
