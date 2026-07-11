#include "result3_1dialog.h"
#include "ui_result3_1dialog.h"

Result3_1Dialog::Result3_1Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Result3_1Dialog)
{
    ui->setupUi(this);
    connect(ui->difficultyUpBtn, SIGNAL(clicked()), this, SLOT(onDifficultyUpClicked()));
    connect(ui->nextLevelBtn, SIGNAL(clicked()), this, SLOT(onNextLevelClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

Result3_1Dialog::~Result3_1Dialog()
{
    delete ui;
}

void Result3_1Dialog::onDifficultyUpClicked()
{
    emit resultSelected(0);
    accept();
}

void Result3_1Dialog::onNextLevelClicked()
{
    emit resultSelected(3);
    accept();
}

void Result3_1Dialog::onBackToMenuClicked()
{
    emit resultSelected(2);
    accept();
}