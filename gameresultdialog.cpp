#include "gameresultdialog.h"

GameResultDialog::GameResultDialog(QWidget *parent)
    : QDialog(parent)
    , isWon(false)
    , currentLevel(1)
    , currentDifficulty(1)
    , skipConfirm(false)   // ★ 新增
{
    setWindowTitle("游戏结果");
    setFixedSize(400, 300);

    QLabel *imageLabel = new QLabel(this);
    imageLabel->setGeometry(10, 10, 380, 180);
    imageLabel->setStyleSheet("background-color: #333333;");

    confirmBtn = new QPushButton("确认", this);
    confirmBtn->setGeometry(150, 200, 100, 40);

    retryBtn = new QPushButton("重新开始", this);
    retryBtn->setGeometry(50, 200, 120, 40);
    retryBtn->hide();

    nextLevelBtn = new QPushButton("难度升级", this);
    nextLevelBtn->setGeometry(130, 200, 140, 40);
    nextLevelBtn->hide();

    goToNextLevelBtn = new QPushButton("下一关", this);
    goToNextLevelBtn->setGeometry(230, 200, 120, 40);
    goToNextLevelBtn->hide();

    backToMenuBtn = new QPushButton("返回主菜单", this);
    backToMenuBtn->setGeometry(130, 250, 140, 40);
    backToMenuBtn->hide();

    connect(confirmBtn, SIGNAL(clicked()), this, SLOT(onConfirmClicked()));
    connect(retryBtn, SIGNAL(clicked()), this, SLOT(onRetryClicked()));
    connect(nextLevelBtn, SIGNAL(clicked()), this, SLOT(onNextLevelClicked()));
    connect(goToNextLevelBtn, SIGNAL(clicked()), this, SLOT(onGoToNextLevelClicked()));
    connect(backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

GameResultDialog::~GameResultDialog()
{
}

// ★ 新增
void GameResultDialog::setSkipConfirm(bool skip)
{
    skipConfirm = skip;
}

void GameResultDialog::setResult(bool won, int level, int difficulty)
{
    isWon = won;
    currentLevel = level;
    currentDifficulty = difficulty;

    confirmBtn->show();
    retryBtn->hide();
    nextLevelBtn->hide();
    goToNextLevelBtn->hide();
    backToMenuBtn->hide();

    // ★ 新增：跳过确认步骤
    if (skipConfirm && isWon) {
        onConfirmClicked();
    }
}

void GameResultDialog::onConfirmClicked()
{
    confirmBtn->hide();

    if (isWon) {
        if (currentDifficulty < 3) {
            nextLevelBtn->setText("难度升级");
            nextLevelBtn->show();

            goToNextLevelBtn->setText("下一关");
            goToNextLevelBtn->show();

            retryBtn->hide();
        } else {
            if (currentLevel < 3) {
                goToNextLevelBtn->setText("下一关");
                goToNextLevelBtn->show();
                nextLevelBtn->hide();
                retryBtn->hide();
            } else {
                backToMenuBtn->show();
            }
        }
    } else {
        // 输了
        retryBtn->setText("重新开始");
        retryBtn->show();

        if (currentDifficulty == 1) {
            // 简单模式：重新开始 或 返回主菜单
            backToMenuBtn->setText("返回主菜单");     // ★ 原本是"返回"，改成"返回主菜单"
            backToMenuBtn->show();
            nextLevelBtn->hide();
            goToNextLevelBtn->hide();
        } else {
            // 中等或困难：重新开始 或 进入下一关卡
            goToNextLevelBtn->setText("进入下一关卡");
            goToNextLevelBtn->show();
            backToMenuBtn->hide();
            nextLevelBtn->hide();
        }
    }
}

void GameResultDialog::onRetryClicked()
{
    accept();
    emit resultSelected(1);
}

void GameResultDialog::onNextLevelClicked()
{
    accept();
    if (isWon) {
        emit resultSelected(0);
    } else {
        emit resultSelected(1);
    }
}

void GameResultDialog::onGoToNextLevelClicked()
{
    accept();
    emit resultSelected(3);
}

void GameResultDialog::onBackToMenuClicked()
{
    accept();
    emit resultSelected(2);
}
