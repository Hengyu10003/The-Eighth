#include "level3startdialog.h"
#include "ui_level3startdialog.h"
#include "game3introdialog.h"

Level3StartDialog::Level3StartDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Level3StartDialog)
{
    ui->setupUi(this);

    connect(ui->startGameBtn, SIGNAL(clicked()), this, SLOT(onStartGameClicked()));
    connect(ui->gameIntroBtn, SIGNAL(clicked()), this, SLOT(onGameIntroClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

Level3StartDialog::~Level3StartDialog()
{
    delete ui;
}

void Level3StartDialog::onStartGameClicked()
{
    accept();
}

void Level3StartDialog::onGameIntroClicked()
{
    Game3IntroDialog dialog(this);
    dialog.exec();
}

void Level3StartDialog::onBackToMenuClicked()
{
    reject();
}