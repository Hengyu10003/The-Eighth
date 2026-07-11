#ifndef BACKGROUNDDIALOG_H
#define BACKGROUNDDIALOG_H

#include <QDialog>
#include "config.h"

namespace Ui {
class BackgroundDialog;
}

class BackgroundDialog : public QDialog
{
    Q_OBJECT

public:
    BackgroundDialog(QWidget *parent = 0);
    ~BackgroundDialog();

private slots:
    void onBackClicked();

private:
    Ui::BackgroundDialog *ui;
};

#endif // BACKGROUNDDIALOG_H
