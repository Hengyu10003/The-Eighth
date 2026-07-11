#ifndef LEVELSTARTDIALOG_H
#define LEVELSTARTDIALOG_H

#include <QDialog>

#include "config.h"

namespace Ui {
class LevelStartDialog;
}

class LevelStartDialog : public QDialog
{
    Q_OBJECT

public:
    LevelStartDialog(QWidget *parent = 0);
    ~LevelStartDialog();

    void setLevel(int level);

signals:
    void startGame(int gameType);

private slots:
    void onStartGameClicked();
    void onGameIntroClicked();
    void onBackToMenuClicked();

private:
    Ui::LevelStartDialog *ui;
    int currentLevel;
};

#endif // LEVELSTARTDIALOG_H
