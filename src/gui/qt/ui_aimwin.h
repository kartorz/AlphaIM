/********************************************************************************
** Form generated from reading UI file 'aimwin.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AIMWIN_H
#define UI_AIMWIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

class Ui_AimWin
{
public:
    QToolButton *lanToolButton;
    QToolButton *punToolButton;
    QToolButton *helpToolButton;

    void setupUi(QDialog *AimWin)
    {
        if (AimWin->objectName().isEmpty())
            AimWin->setObjectName(QStringLiteral("AimWin"));
        AimWin->setWindowModality(Qt::NonModal);
        AimWin->resize(123, 41);
        lanToolButton = new QToolButton(AimWin);
        lanToolButton->setObjectName(QStringLiteral("lanToolButton"));
        lanToolButton->setGeometry(QRect(0, 0, 40, 40));
        QIcon icon;
        icon.addFile(QStringLiteral(":/res/cn.png"), QSize(), QIcon::Normal, QIcon::Off);
        lanToolButton->setIcon(icon);
        lanToolButton->setIconSize(QSize(32, 32));
        punToolButton = new QToolButton(AimWin);
        punToolButton->setObjectName(QStringLiteral("punToolButton"));
        punToolButton->setGeometry(QRect(40, 0, 40, 40));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/res/pun.png"), QSize(), QIcon::Normal, QIcon::Off);
        punToolButton->setIcon(icon1);
        punToolButton->setIconSize(QSize(32, 32));
        helpToolButton = new QToolButton(AimWin);
        helpToolButton->setObjectName(QStringLiteral("helpToolButton"));
        helpToolButton->setGeometry(QRect(80, 0, 40, 40));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/res/help.png"), QSize(), QIcon::Normal, QIcon::Off);
        helpToolButton->setIcon(icon2);
        helpToolButton->setIconSize(QSize(32, 32));

        retranslateUi(AimWin);

        QMetaObject::connectSlotsByName(AimWin);
    } // setupUi

    void retranslateUi(QDialog *AimWin)
    {
        AimWin->setWindowTitle(QString());
        lanToolButton->setText(QString());
        punToolButton->setText(QString());
        helpToolButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class AimWin: public Ui_AimWin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AIMWIN_H
