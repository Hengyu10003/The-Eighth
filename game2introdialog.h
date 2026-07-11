#ifndef GAME2INTRODIALOG_H
#define GAME2INTRODIALOG_H

#include <QDialog>

namespace Ui {
class Game2IntroDialog;
}

class Game2IntroDialog : public QDialog
{
    Q_OBJECT

public:
    Game2IntroDialog(QWidget *parent = 0);
    ~Game2IntroDialog();

private slots:
    void onReturnClicked();

private:
    Ui::Game2IntroDialog *ui;
};

#endif // GAME2INTRODIALOG_H