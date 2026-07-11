#include "backgrounddialog.h"
#include "ui_backgrounddialog.h"

BackgroundDialog::BackgroundDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::BackgroundDialog)
{
    ui->setupUi(this);

    connect(ui->backBtn, SIGNAL(clicked()), this, SLOT(onBackClicked()));
}

BackgroundDialog::~BackgroundDialog()
{
    delete ui;
}

void BackgroundDialog::onBackClicked()
{
    reject();
}
