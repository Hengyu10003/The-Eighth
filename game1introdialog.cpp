#include "game1introdialog.h"
#include "ui_game1introdialog.h"

Game1IntroDialog::Game1IntroDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Game1IntroDialog)
{
    ui->setupUi(this);
    connect(ui->returnBtn, SIGNAL(clicked()), this, SLOT(onReturnClicked()));
}

Game1IntroDialog::~Game1IntroDialog()
{
    delete ui;
}

void Game1IntroDialog::onReturnClicked()
{
    reject();
}