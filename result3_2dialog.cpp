#include "result3_2dialog.h"
#include "ui_result3_2dialog.h"

Result3_2Dialog::Result3_2Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Result3_2Dialog)
{
    ui->setupUi(this);
    connect(ui->difficultyUpBtn, SIGNAL(clicked()), this, SLOT(onDifficultyUpClicked()));
    connect(ui->nextLevelBtn, SIGNAL(clicked()), this, SLOT(onNextLevelClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

Result3_2Dialog::~Result3_2Dialog()
{
    delete ui;
}

void Result3_2Dialog::setResult(bool won)
{
    if (won) {
        ui->difficultyUpBtn->show();
        ui->nextLevelBtn->show();
        ui->backToMenuBtn->hide();
    } else {
        ui->difficultyUpBtn->hide();
        ui->nextLevelBtn->hide();
        ui->backToMenuBtn->show();
    }
}

void Result3_2Dialog::onDifficultyUpClicked()
{
    emit resultSelected(0);
    accept();
}

void Result3_2Dialog::onNextLevelClicked()
{
    emit resultSelected(3);
    accept();
}

void Result3_2Dialog::onBackToMenuClicked()
{
    emit resultSelected(2);
    accept();
}

void Result3_2Dialog::onRetryClicked()
{
    emit resultSelected(1);
    accept();
}