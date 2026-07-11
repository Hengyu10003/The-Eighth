#ifndef RESULT1_2DIALOG_H
#define RESULT1_2DIALOG_H

#include <QDialog>

namespace Ui {
class Result1_2Dialog;
}

class Result1_2Dialog : public QDialog
{
    Q_OBJECT

public:
    Result1_2Dialog(QWidget *parent = 0);
    ~Result1_2Dialog();

signals:
    void resultSelected(int result);

private slots:
    void onDifficultyUpClicked();
    void onNextLevelClicked();
    void onBackToMenuClicked();

private:
    Ui::Result1_2Dialog *ui;
};

#endif // RESULT1_2DIALOG_H