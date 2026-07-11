#ifndef LEVEL1STARTDIALOG_H
#define LEVEL1STARTDIALOG_H

#include <QDialog>

namespace Ui {
class Level1StartDialog;
}

class Level1StartDialog : public QDialog
{
    Q_OBJECT

public:
    Level1StartDialog(QWidget *parent = 0);
    ~Level1StartDialog();

signals:
    void startGame(int gameType);

private slots:
    void onStartGameClicked();
    void onGameIntroClicked();
    void onBackToMenuClicked();

private:
    Ui::Level1StartDialog *ui;
};

#endif // LEVEL1STARTDIALOG_H