#include "result1_3dialog.h"
#include "ui_result1_3dialog.h"

Result1_3Dialog::Result1_3Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Result1_3Dialog)
{
    ui->setupUi(this);
    connect(ui->nextLevelBtn, SIGNAL(clicked()), this, SLOT(onNextLevelClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

Result1_3Dialog::~Result1_3Dialog()
{
    delete ui;
}

void Result1_3Dialog::onNextLevelClicked()
{
    emit resultSelected(3);
    accept();
}

void Result1_3Dialog::onBackToMenuClicked()
{
    emit resultSelected(2);
    accept();
}