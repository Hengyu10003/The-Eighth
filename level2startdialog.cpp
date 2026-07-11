#include "level2startdialog.h"
#include "ui_level2startdialog.h"
#include "game2introdialog.h"

Level2StartDialog::Level2StartDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Level2StartDialog)
{
    ui->setupUi(this);

    connect(ui->startGameBtn, SIGNAL(clicked()), this, SLOT(onStartGameClicked()));
    connect(ui->gameIntroBtn, SIGNAL(clicked()), this, SLOT(onGameIntroClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

Level2StartDialog::~Level2StartDialog()
{
    delete ui;
}

void Level2StartDialog::onStartGameClicked()
{
    accept();
}

void Level2StartDialog::onGameIntroClicked()
{
    Game2IntroDialog dialog(this);
    dialog.exec();
}

void Level2StartDialog::onBackToMenuClicked()
{
    reject();
}