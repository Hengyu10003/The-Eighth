#ifndef RESULT2_1DIALOG_H
#define RESULT2_1DIALOG_H

#include <QDialog>

namespace Ui {
class Result2_1Dialog;
}

class Result2_1Dialog : public QDialog
{
    Q_OBJECT

public:
    Result2_1Dialog(QWidget *parent = 0);
    ~Result2_1Dialog();
    void setResult(bool won);

signals:
    void resultSelected(int result);

private slots:
    void onDifficultyUpClicked();
    void onNextLevelClicked();
    void onBackToMenuClicked();
    void onRetryClicked();

private:
    Ui::Result2_1Dialog *ui;
};

#endif // RESULT2_1DIALOG_H