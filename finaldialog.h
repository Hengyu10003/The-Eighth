#ifndef FINALDIALOG_H
#define FINALDIALOG_H

#include <QDialog>

namespace Ui {
class FinalDialog;
}

class FinalDialog : public QDialog
{
    Q_OBJECT

public:
    FinalDialog(QWidget *parent = 0);
    ~FinalDialog();

signals:
    void backToMenu();//信号

private slots:
    void onBackToMenuClicked();//槽函数

private:
    Ui::FinalDialog *ui;
};

#endif // FINALDIALOG_H
