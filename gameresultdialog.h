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

    void setResult(bool won, int level, int difficulty);
    void setSkipConfirm(bool skip);

signals:
    void resultSelected(int result);

private slots:
    void onConfirmClicked();
    void onRetryClicked();
    void onNextLevelClicked();
    void onGoToNextLevelClicked();
    void onBackToMenuClicked();

private:
    Ui::GameResultDialog *ui;
    bool isWon;
    int currentLevel;
    int currentDifficulty;
    bool skipConfirm;
};

#endif // GAMERESULTDIALOG_H
