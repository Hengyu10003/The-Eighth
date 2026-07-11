#include "result2_3dialog.h"
#include "ui_result2_3dialog.h"

Result2_3Dialog::Result2_3Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Result2_3Dialog)
{
    ui->setupUi(this);
    connect(ui->nextLevelBtn, SIGNAL(clicked()), this, SLOT(onNextLevelClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

Result2_3Dialog::~Result2_3Dialog()
{
    delete ui;
}

void Result2_3Dialog::onNextLevelClicked()
{
    emit resultSelected(3);
    accept();
}

void Result2_3Dialog::onBackToMenuClicked()
{
    emit resultSelected(2);
    accept();
}