#include <QApplication>
#include <QtX11Extras/QX11Info>

#include "aimwin.h"
#include "GuiMessager.h"

#include "aim.h"


int aim_app_main(MessageQueue *q, fun_gui_activate_callback uicallback, int argc, char *argv[])
{
    QApplication a(argc, argv);

    Display *dsy = QX11Info::display();
    uicallback(dsy); // will block system.

    AimWin w;
    w.hide();
    //w.show();

    //QRect rec = QApplication::desktop()->screenGeometry();
    //height = rec.height();
    //width = rec.width();

    //QSystemTrayIcon
    return a.exec();
}
