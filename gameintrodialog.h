#ifndef GAMEINTRODIALOG_H
#define GAMEINTRODIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QLabel>

#include "config.h"

class GameIntroDialog : public QDialog
{
    Q_OBJECT

public:
    GameIntroDialog(QWidget *parent = 0, int level = 1);
    ~GameIntroDialog();

private slots:
    void onBackClicked();

private:
    QPushButton *backBtn;
    int currentLevel;
};

#endif // GAMEINTRODIALOG_H
