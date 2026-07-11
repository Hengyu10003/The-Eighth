#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>

#include "config.h"

class MainWindow;
class GameResultDialog;

class GameController : public QObject
{
    Q_OBJECT

public:
    GameController(MainWindow *mainWindow);
    ~GameController();

private slots:
    void onMazeCompleted();
    void onLinkGameWon();
    void onLinkGameLost();
    void onMarioGameWon();
    void onMarioGameLost();
    void onHollowKnightWon();
    void onHollowKnightLost();
    void onLevelStart(int gameType);
    void onGameResult(int result);


private:
    MainWindow *mainWindow;
    GameResultDialog *gameResultDialog;

    void showLevelStart(int level);
    void handleGameResult(bool won, int level, int difficulty);
    void returnToMainMenu();
    static void saveLeaderboardEntry(const QString &name, int fragments, int time);
};

#endif // GAMECONTROLLER_H
