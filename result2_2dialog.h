#ifndef RESULT2_2DIALOG_H
#define RESULT2_2DIALOG_H

#include <QDialog>

namespace Ui {
class Result2_2Dialog;
}

class Result2_2Dialog : public QDialog
{
    Q_OBJECT

public:
    Result2_2Dialog(QWidget *parent = 0);
    ~Result2_2Dialog();
    void setResult(bool won);

signals:
    void resultSelected(int result);

private slots:
    void onDifficultyUpClicked();
    void onNextLevelClicked();
    void onBackToMenuClicked();
    void onRetryClicked();

private:
    Ui::Result2_2Dialog *ui;
};

#endif // RESULT2_2DIALOG_H