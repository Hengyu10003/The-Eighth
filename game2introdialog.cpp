#include "game2introdialog.h"
#include "ui_game2introdialog.h"

Game2IntroDialog::Game2IntroDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Game2IntroDialog)
{
    ui->setupUi(this);
    connect(ui->returnBtn, SIGNAL(clicked()), this, SLOT(onReturnClicked()));
}

Game2IntroDialog::~Game2IntroDialog()
{
    delete ui;
}

void Game2IntroDialog::onReturnClicked()
{
    reject();
}