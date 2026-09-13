#include "finaldialog.h"
#include "ui_finaldialog.h"

FinalDialog::FinalDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FinalDialog)
{
    ui->setupUi(this);
    connect(ui->backToMenuBtn, SIGNAL(clicked()), this, SLOT(onBackToMenuClicked()));
}

FinalDialog::~FinalDialog()
{
    delete ui;
}

void FinalDialog::onBackToMenuClicked()
{
    emit backToMenu();//发射这个信号，表示要返回菜单了
    accept();//关闭弹窗，并返回
}
