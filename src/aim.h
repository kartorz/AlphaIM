#ifndef _AIM_H_
#define _AIM_H_

#include <string>
#include "config.h"

#ifdef X11
#include <X11/Xlib.h>
typedef void (*fun_gui_activate_callback)(Window imwin, Display *dpy);
#endif

#define ICONS_PATH  "theme/hicolor/32x32"
#define MAX_WORK_THREAD  2
#define MAX_PREEDIT_PAGE 5


#ifdef AL_DEBUG
#define PRINTF(fmt, args...)  printf(fmt, ##args)
#else
#define PRINTF(fmt, args...)
#endif

/* i in [0 .. c-1] */
#define LOOP(size)  for (int i=0; i<size; i++)

extern std::string system_dir;
extern std::string home_dir;

#endif
