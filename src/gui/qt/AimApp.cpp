#include <QApplication>
#include <QtX11Extras/QX11Info>

#include "aimwin.h"
#include "GuiMessager.h"

#include "aim.h"


int aim_app_main(MessageQueue *q, fun_gui_activate_callback uicallback, int argc, char *argv[])
{
    QApplication a(argc, argv);

    AimWin w;
    w.hide();

    //Display *dsy = QX11Info::display();
    //uicallback(dsy); // will block system.
    
    //QRect rec = QApplication::desktop()->screenGeometry();
    //height = rec.height();
    //width = rec.width();

    //QSystemTrayIcon
    
    return a.exec();
}
