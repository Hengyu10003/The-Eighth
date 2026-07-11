#ifndef RESULT3_3DIALOG_H
#define RESULT3_3DIALOG_H

#include <QDialog>

namespace Ui {
class Result3_3Dialog;
}

class Result3_3Dialog : public QDialog
{
    Q_OBJECT

public:
    Result3_3Dialog(QWidget *parent = 0);
    ~Result3_3Dialog();
    void setResult(bool won);

signals:
    void resultSelected(int result);

private slots:
    void onNextLevelClicked();
    void onRetryClicked();

private:
    Ui::Result3_3Dialog *ui;
};

#endif // RESULT3_3DIALOG_H