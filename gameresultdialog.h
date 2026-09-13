#ifndef GAMERESULTDIALOG_H
#define GAMERESULTDIALOG_H

#include <QDialog>

namespace Ui {
class GameResultDialog;
}

class GameResultDialog : public QDialog
{
    Q_OBJECT

public:
    GameResultDialog(QWidget *parent = 0);
    ~GameResultDialog();

    void setResult(int level, int difficulty);

signals:
    void resultSelected(int result);

private slots:
    void onRetryClicked();
    void onBackToMenuClicked();

private:
    Ui::GameResultDialog *ui;
    int currentLevel;
    int currentDifficulty;
};

#endif // GAMERESULTDIALOG_H
