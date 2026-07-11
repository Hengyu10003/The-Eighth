#ifndef RESULT3_2DIALOG_H
#define RESULT3_2DIALOG_H

#include <QDialog>

namespace Ui {
class Result3_2Dialog;
}

class Result3_2Dialog : public QDialog
{
    Q_OBJECT

public:
    Result3_2Dialog(QWidget *parent = 0);
    ~Result3_2Dialog();

signals:
    void resultSelected(int result);

private slots:
    void onDifficultyUpClicked();
    void onNextLevelClicked();
    void onBackToMenuClicked();

private:
    Ui::Result3_2Dialog *ui;
};

#endif // RESULT3_2DIALOG_H