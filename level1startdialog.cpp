#include "level1startdialog.h"
#include "ui_level1startdialog.h"
#include "game1introdialog.h"

Level1StartDialog::Level1StartDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Level1StartDialog)
{
    ui->setupUi(this);

    connect(ui->startGameBtn, SIGNAL(clicked()), this, SLOT(onStartGameClicked()));
    connect(ui->gameIntroBtn, SIGNAL(clicked()), this, SLOT(onGameIntroClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

Level1StartDialog::~Level1StartDialog()
{
    delete ui;
}

void Level1StartDialog::onStartGameClicked()
{
    accept();
}

void Level1StartDialog::onGameIntroClicked()
{
    Game1IntroDialog dialog(this);
    dialog.exec();
}

void Level1StartDialog::onBackToMenuClicked()
{
    reject();
}