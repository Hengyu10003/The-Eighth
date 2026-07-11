#ifndef GAME3INTRODIALOG_H
#define GAME3INTRODIALOG_H

#include <QDialog>

namespace Ui {
class Game3IntroDialog;
}

class Game3IntroDialog : public QDialog
{
    Q_OBJECT

public:
    Game3IntroDialog(QWidget *parent = 0);
    ~Game3IntroDialog();

private slots:
    void onReturnClicked();

private:
    Ui::Game3IntroDialog *ui;
};

#endif // GAME3INTRODIALOG_H