#include "result3_3dialog.h"
#include "ui_result3_3dialog.h"

Result3_3Dialog::Result3_3Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Result3_3Dialog)
{
    ui->setupUi(this);
    connect(ui->nextLevelBtn, SIGNAL(clicked()), this, SLOT(onNextLevelClicked()));
}

Result3_3Dialog::~Result3_3Dialog()
{
    delete ui;
}

void Result3_3Dialog::setResult(bool won)
{
    if (won) {
        ui->nextLevelBtn->show();
    } else {
        ui->nextLevelBtn->hide();
    }
}

void Result3_3Dialog::onNextLevelClicked()
{
    emit resultSelected(3);
    accept();
}

void Result3_3Dialog::onRetryClicked()
{
    emit resultSelected(1);
    accept();
}