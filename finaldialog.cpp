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

void FinalDialog::setResult(const QString &text)
{
//    ui->textBrowser->setText(text);
}

void FinalDialog::onBackToMenuClicked()
{
    emit backToMenu();
    accept();
}
