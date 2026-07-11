#include "gameintrodialog.h"

GameIntroDialog::GameIntroDialog(QWidget *parent, int level)
    : QDialog(parent)
    , currentLevel(level)
{
    setWindowTitle("游戏介绍");
    setFixedSize(500, 400);

    QLabel *imageLabel = new QLabel(this);
    imageLabel->setGeometry(10, 10, 480, 320);
    imageLabel->setStyleSheet("background-color: #333333;");

    backBtn = new QPushButton("返回", this);
    backBtn->setGeometry(200, 340, 100, 40);

    connect(backBtn, SIGNAL(clicked()), this, SLOT(onBackClicked()));
}

GameIntroDialog::~GameIntroDialog()
{
}

void GameIntroDialog::onBackClicked()
{
    reject();
}
