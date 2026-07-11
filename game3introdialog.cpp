#include "game3introdialog.h"
#include "ui_game3introdialog.h"

Game3IntroDialog::Game3IntroDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Game3IntroDialog)
{
    ui->setupUi(this);
    connect(ui->returnBtn, SIGNAL(clicked()), this, SLOT(onReturnClicked()));
}

Game3IntroDialog::~Game3IntroDialog()
{
    delete ui;
}

void Game3IntroDialog::onReturnClicked()
{
    reject();
}