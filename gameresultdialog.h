#ifndef GAMERESULTDIALOG_H
#define GAMERESULTDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>

#include "config.h"

class GameResultDialog : public QDialog
{
    Q_OBJECT

public:
    GameResultDialog(QWidget *parent = 0);
    ~GameResultDialog();

    void setResult(bool won, int level, int difficulty);
    void setSkipConfirm(bool skip);  // ★ 新增

signals:
    void resultSelected(int result);

private slots:
    void onRetryClicked();
    void onNextLevelClicked();
    void onGoToNextLevelClicked();
    void onBackToMenuClicked();
    void onConfirmClicked();

private:
    QPushButton *retryBtn;
    QPushButton *nextLevelBtn;
    QPushButton *goToNextLevelBtn;
    QPushButton *backToMenuBtn;
    QPushButton *confirmBtn;

    bool isWon;
    int currentLevel;
    int currentDifficulty;
    bool skipConfirm;  // ★ 新增
};

#endif // GAMERESULTDIALOG_H
