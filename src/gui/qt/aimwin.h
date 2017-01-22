#ifndef _AIMWIN_H_
#define _AIMWIN_H_

#include <QtWidgets/QDialog>
#include <QtWidgets/QSystemTrayIcon>
#include "ui_aimwin.h"

class IcWin;

namespace Ui {
class AimWin;
}

class AimWin : public QDialog
{
    Q_OBJECT

public:
    explicit AimWin(QWidget *parent = 0);
    ~AimWin();

private slots:
    void on_lanToolButton_clicked();

    void on_punToolButton_clicked();

    void on_helpToolButton_clicked();

    void onSystrayActivated(QSystemTrayIcon::ActivationReason reason);

    void onMesssagerIMOn();

    void onMesssagerIMOff();

    void onMesssagerIMClose();

    void onMesssagerIMCommit();

    void onMesssagerIMLan(bool isCN);

    void onMesssagerIMPun(bool isCN);

    void onMesssagerIMPreedit(int x, int y, int w, int h,
                                std::string strInput,
                                void *user_data);

private:
    Ui::AimWin *ui;
    IcWin*  ic;
    QSystemTrayIcon* m_systray;
};

#endif // AIMWIN_H
