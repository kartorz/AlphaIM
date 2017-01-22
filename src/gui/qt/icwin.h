#ifndef _ICWIN_H_
#define _ICWIN_H_

#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include "ui_icwin.h"

#include <deque>
#include "aim.h"
#include "iIM.h"

namespace Ui {
class IcWin;
}

#define PREEDIT_ITEMS_MAX    IM_ITEM_PAGE_SIZE

class IcWin : public QDialog
{
    Q_OBJECT

public:
    explicit IcWin(QWidget *parent = 0);
    ~IcWin();

    void showPreedit(int x, int y, int w, int h,
                     std::string& strInput,
                     std::deque<IMItem> *items);
private:
    QLabel* preeditLabels[PREEDIT_ITEMS_MAX];
    Ui::IcWin *ui;
};

#endif // ICWIN_H
