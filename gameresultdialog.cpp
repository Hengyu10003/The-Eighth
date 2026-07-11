#include "gameresultdialog.h"
#include "ui_gameresultdialog.h"

GameResultDialog::GameResultDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GameResultDialog)
    , isWon(false)
    , currentLevel(1)
    , currentDifficulty(1)
{
    ui->setupUi(this);

    connect(ui->retryBtn, SIGNAL(clicked()), this, SLOT(onRetryClicked()));
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

GameResultDialog::~GameResultDialog()
{
    delete ui;
}

void GameResultDialog::setResult(bool won, int level, int difficulty)
{
    isWon = won;
    currentLevel = level;
    currentDifficulty = difficulty;

    // 失败界面：只显示"重新开始"和"返回主菜单"
    Q_UNUSED(isWon);
    ui->retryBtn->show();
    ui->backToMenuBtn->show();
}

void GameResultDialog::onRetryClicked()
{
    accept();
    emit resultSelected(1);
}

void GameResultDialog::onBackToMenuClicked()
{
    accept();
    emit resultSelected(2);
}
