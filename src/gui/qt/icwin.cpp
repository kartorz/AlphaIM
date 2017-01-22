#include "icwin.h"
#include "iIM.h"

IcWin::IcWin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IcWin)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
        
    preeditLabels[0] = ui->label1;
    preeditLabels[1] = ui->label2;
    preeditLabels[2] = ui->label3;
    preeditLabels[3] = ui->label4;
    preeditLabels[4] = ui->label5;
    preeditLabels[5] = ui->label6;
    preeditLabels[6] = ui->label7;
    preeditLabels[7] = ui->label8;
    preeditLabels[8] = ui->label9;
}

IcWin::~IcWin()
{
    delete ui;
}

void IcWin::showPreedit(int x, int y, int w, int h,
                    std::string& strInput,
                    std::deque<IMItem> *items)
{    
    QString markup = "<font color=\"blue\">";
    markup += QString::fromUtf8(strInput.c_str());
    markup += "</font>";
    ui->labelInput->setText(markup);
    int pi = 0;
    if (items != NULL) {;
        int len = items->size() < PREEDIT_ITEMS_MAX ? items->size() : PREEDIT_ITEMS_MAX;
        for (; pi < len ; pi++) {
            std::string text = "  ";
            char pichr = 0x31 + pi;
            text += pichr;
            text += ": ";
            text += items->at(pi).val;
            //gtk_label_set_text(GTK_LABEL (klass->preedit_label[pi]), text.c_str());
            if (pi == 0) {
                markup = "<font color=\"green\">" + QString::fromUtf8(text.c_str()) +  "</font>";
            } else {
                markup = "<font color=\"blue\">"  + QString::fromUtf8(text.c_str()) +  "</font>";
            }
            preeditLabels[pi]->setText(markup);
        }

        delete items;
    }

    for (; pi < PREEDIT_ITEMS_MAX; pi++)
         preeditLabels[pi]->clear();

    if (isHidden())
        show();
    raise();
    activateWindow();
}
