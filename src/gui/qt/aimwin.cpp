#include "aimwin.h"

#include <deque>

#include "icwin.h"
#include "aim.h"
#include "iIM.h"

AimWin::AimWin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AimWin)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

    m_systray = new QSystemTrayIcon(this);
    QIcon icon;
    icon.addFile(QStringLiteral(":/res/app.png"), QSize(), QIcon::Normal, QIcon::Off);
    m_systray->setIcon(icon);
    m_systray->show();
    QObject::connect(m_systray,
            SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this,
            SLOT(onSystrayActivated(QSystemTrayIcon::ActivationReason)));

    ic = new IcWin(this);
}

AimWin::~AimWin()
{
    delete ui;
    delete ic;
}

void AimWin::onSystrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    qDebug("joni onSystrayActivated\n");
    if (reason == QSystemTrayIcon::Trigger) {
        m_systray->hide();
        activateWindow();
        showNormal();
    }
}

void AimWin::on_lanToolButton_clicked()
{

}

void AimWin::on_punToolButton_clicked()
{

}

void AimWin::on_helpToolButton_clicked()
{

}

void AimWin::onMesssagerIMOn()
{    
}

void AimWin::onMesssagerIMOff()
{
    if (!ic->isHidden())
        ic->close();
}

void AimWin::onMesssagerIMClose()
{
    if (!ic->isHidden())
         ic->close();
}

void AimWin::onMesssagerIMCommit()
{
    if (!ic->isHidden())
         ic->close();
}

void AimWin::onMesssagerIMLan(bool isCN)
{
}

void AimWin::onMesssagerIMPun(bool isCN)
{
}

void AimWin::onMesssagerIMPreedit(int x, int y, int w, int h,
                                    std::string strInput,
                                    void *user_data)
{
    
    std::deque<IMItem> *items = (std::deque<IMItem> *)(user_data);
    ic->showPreedit(x, y, w, h, strInput, items);
}
