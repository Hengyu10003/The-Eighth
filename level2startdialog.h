#ifndef LEVEL2STARTDIALOG_H
#define LEVEL2STARTDIALOG_H

#include <QDialog>

namespace Ui {
class Level2StartDialog;
}

class Level2StartDialog : public QDialog
{
    Q_OBJECT

public:
    Level2StartDialog(QWidget *parent = 0);
    ~Level2StartDialog();

signals:
    void startGame(int gameType);

private slots:
    void onStartGameClicked();
    void onGameIntroClicked();
    void onBackToMenuClicked();

private:
    Ui::Level2StartDialog *ui;
};

#endif // LEVEL2STARTDIALOG_H