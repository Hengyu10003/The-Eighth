#ifndef GAME1INTRODIALOG_H
#define GAME1INTRODIALOG_H

#include <QDialog>

namespace Ui {
class Game1IntroDialog;
}

class Game1IntroDialog : public QDialog
{
    Q_OBJECT

public:
    Game1IntroDialog(QWidget *parent = 0);
    ~Game1IntroDialog();

private slots:
    void onReturnClicked();

private:
    Ui::Game1IntroDialog *ui;
};

#endif // GAME1INTRODIALOG_H