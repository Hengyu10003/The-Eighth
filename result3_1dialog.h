#ifndef RESULT3_1DIALOG_H
#define RESULT3_1DIALOG_H

#include <QDialog>

namespace Ui {
class Result3_1Dialog;
}

class Result3_1Dialog : public QDialog
{
    Q_OBJECT

public:
    Result3_1Dialog(QWidget *parent = 0);
    ~Result3_1Dialog();
    void setResult(bool won);

signals:
    void resultSelected(int result);

private slots:
    void onDifficultyUpClicked();
    void onNextLevelClicked();
    void onBackToMenuClicked();
    void onRetryClicked();

private:
    Ui::Result3_1Dialog *ui;
};

#endif // RESULT3_1DIALOG_H