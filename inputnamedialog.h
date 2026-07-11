#ifndef INPUTNAMEDIALOG_H
#define INPUTNAMEDIALOG_H

#include <QDialog>

#include "config.h"

namespace Ui {
class InputNameDialog;
}

class InputNameDialog : public QDialog
{
    Q_OBJECT

public:
    InputNameDialog(QWidget *parent = 0);
    ~InputNameDialog();

    QString getPlayerName() const;

private slots:
    void onConfirmClicked();
    void onCancelClicked();

private:
    Ui::InputNameDialog *ui;
    QString playerName;
};

#endif // INPUTNAMEDIALOG_H
