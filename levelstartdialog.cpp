#include "levelstartdialog.h"
#include "ui_levelstartdialog.h"
#include "gameintrodialog.h"

LevelStartDialog::LevelStartDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LevelStartDialog)
    , currentLevel(1)
{
    ui->setupUi(this);

    connect(ui->startGameBtn, SIGNAL(clicked()), this, SLOT(onStartGameClicked()));
    connect(ui->gameIntroBtn, SIGNAL(clicked()), this, SLOT(onGameIntroClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

LevelStartDialog::~LevelStartDialog()
{
    delete ui;
}

void LevelStartDialog::setLevel(int level)
{
    currentLevel = level;
    ui->titleLabel->setText("关卡" + QString::number(level));
}

void LevelStartDialog::onStartGameClicked()
{
    accept();
}

void LevelStartDialog::onGameIntroClicked()
{
    GameIntroDialog dialog(this, currentLevel);
    dialog.exec();
}

void LevelStartDialog::onBackToMenuClicked()
{
    reject();
}
