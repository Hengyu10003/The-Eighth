#include "inputnamedialog.h"
#include "ui_inputnamedialog.h"

InputNameDialog::InputNameDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::InputNameDialog)
{
    ui->setupUi(this);

    connect(ui->confirmBtn, SIGNAL(clicked()), this, SLOT(onConfirmClicked()));
    connect(ui->cancelBtn, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
}

InputNameDialog::~InputNameDialog()
{
    delete ui;
}

QString InputNameDialog::getPlayerName() const
{
    return playerName;
}

void InputNameDialog::onConfirmClicked()
{
    playerName = ui->nameInput->text();
    if (!playerName.isEmpty()) {
        accept();
    }
}

void InputNameDialog::onCancelClicked()
{
    reject();
}
