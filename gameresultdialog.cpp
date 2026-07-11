#include "gameresultdialog.h"
#include "ui_gameresultdialog.h"

GameResultDialog::GameResultDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GameResultDialog)
    , isWon(false)
    , currentLevel(1)
    , currentDifficulty(1)
    , skipConfirm(false)
{
    ui->setupUi(this);

    connect(ui->confirmBtn, SIGNAL(clicked()), this, SLOT(onConfirmClicked()));
    connect(ui->retryBtn, SIGNAL(clicked()), this, SLOT(onRetryClicked()));
    connect(ui->nextLevelBtn, SIGNAL(clicked()), this, SLOT(onNextLevelClicked()));
    connect(ui->goToNextLevelBtn, SIGNAL(clicked()), this, SLOT(onGoToNextLevelClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

GameResultDialog::~GameResultDialog()
{
    delete ui;
}

void GameResultDialog::setSkipConfirm(bool skip)
{
    skipConfirm = skip;
}

void GameResultDialog::setResult(bool won, int level, int difficulty)
{
    isWon = won;
    currentLevel = level;
    currentDifficulty = difficulty;

    ui->confirmBtn->show();
    ui->retryBtn->hide();
    ui->nextLevelBtn->hide();
    ui->goToNextLevelBtn->hide();
    ui->backToMenuBtn->hide();

    if (skipConfirm && isWon) {
        onConfirmClicked();
    }
}

void GameResultDialog::onConfirmClicked()
{
    ui->confirmBtn->hide();

    if (isWon) {
        if (currentDifficulty < 3) {
            ui->nextLevelBtn->setText("难度升级");
            ui->nextLevelBtn->show();
            ui->goToNextLevelBtn->setText("下一关");
            ui->goToNextLevelBtn->show();
            ui->retryBtn->hide();
        } else {
            if (currentLevel < 3) {
                ui->goToNextLevelBtn->setText("下一关");
                ui->goToNextLevelBtn->show();
                ui->nextLevelBtn->hide();
                ui->retryBtn->hide();
            } else {
                ui->backToMenuBtn->show();
            }
        }
    } else {
        ui->retryBtn->setText("重新开始");
        ui->retryBtn->show();

        if (currentDifficulty == 1) {
            ui->backToMenuBtn->setText("返回主菜单");
            ui->backToMenuBtn->show();
            ui->nextLevelBtn->hide();
            ui->goToNextLevelBtn->hide();
        } else {
            ui->goToNextLevelBtn->setText("进入下一关卡");
            ui->goToNextLevelBtn->show();
            ui->backToMenuBtn->hide();
            ui->nextLevelBtn->hide();
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
    emit resultSelected(0);
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
