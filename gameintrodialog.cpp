#include "gameintrodialog.h"
#include "ui_gameintrodialog.h"

GameIntroDialog::GameIntroDialog(QWidget *parent, int level)
    : QDialog(parent)
    , ui(new Ui::GameIntroDialog)
    , currentLevel(level)
{
    ui->setupUi(this);
    connect(ui->backBtn, SIGNAL(clicked()), this, SLOT(onBackClicked()));
}

GameIntroDialog::~GameIntroDialog()
{
    delete ui;
}

void GameIntroDialog::onBackClicked()
{
    reject();
}
