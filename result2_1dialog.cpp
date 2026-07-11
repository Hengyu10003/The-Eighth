#include "result2_1dialog.h"
#include "ui_result2_1dialog.h"

Result2_1Dialog::Result2_1Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Result2_1Dialog)
{
    ui->setupUi(this);
    connect(ui->difficultyUpBtn, SIGNAL(clicked()), this, SLOT(onDifficultyUpClicked()));
    connect(ui->nextLevelBtn, SIGNAL(clicked()), this, SLOT(onNextLevelClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

Result2_1Dialog::~Result2_1Dialog()
{
    delete ui;
}

void Result2_1Dialog::onDifficultyUpClicked()
{
    emit resultSelected(0);
    accept();
}

void Result2_1Dialog::onNextLevelClicked()
{
    emit resultSelected(3);
    accept();
}

void Result2_1Dialog::onBackToMenuClicked()
{
    emit resultSelected(2);
    accept();
}