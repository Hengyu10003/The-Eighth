#ifndef LEVEL3STARTDIALOG_H
#define LEVEL3STARTDIALOG_H

#include <QDialog>

namespace Ui {
class Level3StartDialog;
}

class Level3StartDialog : public QDialog
{
    Q_OBJECT

public:
    Level3StartDialog(QWidget *parent = 0);
    ~Level3StartDialog();

signals:
    void startGame(int gameType);

private slots:
    void onStartGameClicked();
    void onGameIntroClicked();
    void onBackToMenuClicked();

private:
    Ui::Level3StartDialog *ui;
};

#endif // LEVEL3STARTDIALOG_H