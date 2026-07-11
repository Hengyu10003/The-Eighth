#ifndef GAMEINTRODIALOG_H
#define GAMEINTRODIALOG_H

#include <QDialog>

namespace Ui {
class GameIntroDialog;
}

class GameIntroDialog : public QDialog
{
    Q_OBJECT

public:
    GameIntroDialog(QWidget *parent = 0, int level = 1);
    ~GameIntroDialog();

private slots:
    void onBackClicked();

private:
    Ui::GameIntroDialog *ui;
    int currentLevel;
};

#endif // GAMEINTRODIALOG_H
