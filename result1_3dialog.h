#ifndef RESULT1_3DIALOG_H
#define RESULT1_3DIALOG_H

#include <QDialog>

namespace Ui {
class Result1_3Dialog;
}

class Result1_3Dialog : public QDialog
{
    Q_OBJECT

public:
    Result1_3Dialog(QWidget *parent = 0);
    ~Result1_3Dialog();

signals:
    void resultSelected(int result);

private slots:
    void onNextLevelClicked();
    void onBackToMenuClicked();

private:
    Ui::Result1_3Dialog *ui;
};

#endif // RESULT1_3DIALOG_H