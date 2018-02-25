#ifndef _AIM_H_
#define _AIM_H_

#include <string>
#include "config.h"

#define VERSION  "1.2"

#define IM_NAME "AlphaIM"

#define ICONS_PATH  "theme/hicolor/32x32"
#define MAX_WORK_THREAD  2
#define MAX_PREEDIT_PAGE 5

//#define PRINTF(fmt, args...)  printf(fmt, ##args)
#define PRINTF(fmt, args...)

/* i in [0 .. c-1] */
#define LOOP(size)  for (int i=0; i<size; i++)

#define AIM_SRV_NAME  "org.freedesktop.AlphaIM"
#define AIM_SRV_PATH  "/org/freedesktop/AlphaIM"
#define AIM_SRV_INTF  "org.freedesktop.AlphaIM"

#define AIM_NOTIFY_PATH  "/org/freedesktop/AlphaIM/Event"
#define AIM_NOTIFY_INTF  "org.freedesktop.AlphaIM.Event"
#define AIM_NOTIFY_MESSAGE     "Message"

#define AIM_SRV_QIM_PATH  "/org/freedesktop/AlphaIM/qim"
#define AIM_SRV_QIM_INTF   "org.freedesktop.AlphaIM.InputContext"

enum {
    // [iArg1 .. fArg2]: rect.
    // strArg1:          input.
    // pArg1:            items: item being ""  - Don't set keeping the value.
    //                               being " " - clear the old value.

    MSG_IM_INPUT = 0,

    MSG_IM_ON,
    MSG_IM_OFF,
    MSG_IM_CN,
    MSG_IM_EN,
    MSG_IM_CPUN,
    MSG_IM_EPUN,
    MSG_IM_CLOSE,
    MSG_IM_COMMIT,

    MSG_UI_LAN,
    MSG_UI_PUN,

    MSG_QUIT,
};

#endif
