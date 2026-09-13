#include "gameresultdialog.h"
#include "ui_gameresultdialog.h"

GameResultDialog::GameResultDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GameResultDialog)
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

void GameResultDialog::setResult(int level, int difficulty)
{
    currentLevel = level;
    currentDifficulty = difficulty;

    // 失败界面：显示"重新开始"和"返回主菜单"
    ui->retryBtn->show();
    ui->backToMenuBtn->show();
}


//按钮
//重新开始
void GameResultDialog::onRetryClicked()
{
    accept();
    emit resultSelected(1);//释放信号
}
//返回菜单
void GameResultDialog::onBackToMenuClicked()
{
    accept();
    emit resultSelected(2);
}
