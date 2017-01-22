/********************************************************************************
** Form generated from reading UI file 'icwin.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ICWIN_H
#define UI_ICWIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_IcWin
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QLabel *labelInput;
    QLabel *label1;
    QLabel *label2;
    QLabel *label3;
    QLabel *label4;
    QLabel *label5;
    QLabel *label6;
    QLabel *label7;
    QLabel *label8;
    QLabel *label9;

    void setupUi(QDialog *IcWin)
    {
        if (IcWin->objectName().isEmpty())
            IcWin->setObjectName(QStringLiteral("IcWin"));
        IcWin->setWindowModality(Qt::NonModal);
        IcWin->resize(960, 72);
        centralwidget = new QWidget(IcWin);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        labelInput = new QLabel(centralwidget);
        labelInput->setObjectName(QStringLiteral("labelInput"));

        gridLayout->addWidget(labelInput, 0, 0, 1, 9);

        label1 = new QLabel(centralwidget);
        label1->setObjectName(QStringLiteral("label1"));

        gridLayout->addWidget(label1, 1, 0, 1, 1);

        label2 = new QLabel(centralwidget);
        label2->setObjectName(QStringLiteral("label2"));

        gridLayout->addWidget(label2, 1, 1, 1, 1);

        label3 = new QLabel(centralwidget);
        label3->setObjectName(QStringLiteral("label3"));

        gridLayout->addWidget(label3, 1, 2, 1, 1);

        label4 = new QLabel(centralwidget);
        label4->setObjectName(QStringLiteral("label4"));

        gridLayout->addWidget(label4, 1, 3, 1, 1);

        label5 = new QLabel(centralwidget);
        label5->setObjectName(QStringLiteral("label5"));

        gridLayout->addWidget(label5, 1, 4, 1, 1);

        label6 = new QLabel(centralwidget);
        label6->setObjectName(QStringLiteral("label6"));

        gridLayout->addWidget(label6, 1, 5, 1, 1);

        label7 = new QLabel(centralwidget);
        label7->setObjectName(QStringLiteral("label7"));

        gridLayout->addWidget(label7, 1, 6, 1, 1);

        label8 = new QLabel(centralwidget);
        label8->setObjectName(QStringLiteral("label8"));

        gridLayout->addWidget(label8, 1, 7, 1, 1);

        label9 = new QLabel(centralwidget);
        label9->setObjectName(QStringLiteral("label9"));

        gridLayout->addWidget(label9, 1, 8, 1, 1);


        retranslateUi(IcWin);

        QMetaObject::connectSlotsByName(IcWin);
    } // setupUi

    void retranslateUi(QDialog *IcWin)
    {
        IcWin->setWindowTitle(QApplication::translate("IcWin", "MainWindow", 0));
        labelInput->setText(QString());
        label1->setText(QApplication::translate("IcWin", "TextLabel", 0));
        label2->setText(QApplication::translate("IcWin", "TextLabel", 0));
        label3->setText(QApplication::translate("IcWin", "TextLabel", 0));
        label4->setText(QApplication::translate("IcWin", "TextLabel", 0));
        label5->setText(QApplication::translate("IcWin", "TextLabel", 0));
        label6->setText(QApplication::translate("IcWin", "TextLabel", 0));
        label7->setText(QApplication::translate("IcWin", "TextLabel", 0));
        label8->setText(QApplication::translate("IcWin", "TextLabel", 0));
        label9->setText(QApplication::translate("IcWin", "TextLabel", 0));
    } // retranslateUi

};

namespace Ui {
    class IcWin: public Ui_IcWin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ICWIN_H
