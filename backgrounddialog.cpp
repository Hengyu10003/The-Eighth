#include "backgrounddialog.h"
#include "ui_backgrounddialog.h"

BackgroundDialog::BackgroundDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::BackgroundDialog)
{
    ui->setupUi(this);
    //信号发送者，发送的信号，信号接收者，槽函数
    connect(ui->backBtn, SIGNAL(clicked()), this, SLOT(onBackClicked()));
}

BackgroundDialog::~BackgroundDialog()
{
    delete ui;
}

void BackgroundDialog::onBackClicked()
{
    reject();//关闭弹窗并返回
}
