#ifndef RESULT1_1DIALOG_H
#define RESULT1_1DIALOG_H

#include <QDialog>

namespace Ui {
class Result1_1Dialog;
}

class Result1_1Dialog : public QDialog
{
    Q_OBJECT

public:
    Result1_1Dialog(QWidget *parent = 0);
    ~Result1_1Dialog();
    void setResult(bool won);

signals:
    void resultSelected(int result);

private slots:
    void onDifficultyUpClicked();
    void onNextLevelClicked();
    void onBackToMenuClicked();
    void onRetryClicked();

private:
    Ui::Result1_1Dialog *ui;
};

#endif // RESULT1_1DIALOG_H