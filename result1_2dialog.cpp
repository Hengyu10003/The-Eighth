#include "result1_2dialog.h"
#include "ui_result1_2dialog.h"

Result1_2Dialog::Result1_2Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Result1_2Dialog)
{
    ui->setupUi(this);
    connect(ui->difficultyUpBtn, SIGNAL(clicked()), this, SLOT(onDifficultyUpClicked()));
    connect(ui->nextLevelBtn, SIGNAL(clicked()), this, SLOT(onNextLevelClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

Result1_2Dialog::~Result1_2Dialog()
{
    delete ui;
}

void Result1_2Dialog::onDifficultyUpClicked()
{
    emit resultSelected(0);
    accept();
}

void Result1_2Dialog::onNextLevelClicked()
{
    emit resultSelected(3);
    accept();
}

void Result1_2Dialog::onBackToMenuClicked()
{
    emit resultSelected(2);
    accept();
}