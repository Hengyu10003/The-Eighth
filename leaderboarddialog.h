#ifndef LEADERBOARDDIALOG_H
#define LEADERBOARDDIALOG_H

#include <QDialog>
#include <QList>
#include <QString>

namespace Ui {
class LeaderboardDialog;
}

class LeaderboardDialog : public QDialog
{
    Q_OBJECT

public:
    LeaderboardDialog(QWidget *parent = 0);
    ~LeaderboardDialog();

    void loadLeaderboard();
    void saveLeaderboard();
    void addScore(const QString &name, int fragments, int time);

private slots:
    void onBackClicked();

private:
    Ui::LeaderboardDialog *ui;

    struct Score {
        QString name;
        int fragments;
        int time;
    };

    QList<Score> scores;
};

#endif // LEADERBOARDDIALOG_H
