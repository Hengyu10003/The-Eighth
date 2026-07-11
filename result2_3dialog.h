#ifndef RESULT2_3DIALOG_H
#define RESULT2_3DIALOG_H

#include <QDialog>

namespace Ui {
class Result2_3Dialog;
}

class Result2_3Dialog : public QDialog
{
    Q_OBJECT

public:
    Result2_3Dialog(QWidget *parent = 0);
    ~Result2_3Dialog();
    void setResult(bool won);

signals:
    void resultSelected(int result);

private slots:
    void onNextLevelClicked();
    void onBackToMenuClicked();
    void onRetryClicked();

private:
    Ui::Result2_3Dialog *ui;
};

#endif // RESULT2_3DIALOG_H