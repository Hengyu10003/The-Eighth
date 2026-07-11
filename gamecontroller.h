#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>

#include "config.h"

class MainWindow;
class Level1StartDialog;
class Level2StartDialog;
class Level3StartDialog;
class Result1_1Dialog;
class Result1_2Dialog;
class Result1_3Dialog;
class Result2_1Dialog;
class Result2_2Dialog;
class Result2_3Dialog;
class Result3_1Dialog;
class Result3_2Dialog;
class Result3_3Dialog;

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
    void onLevel1Start();
    void onLevel2Start();
    void onLevel3Start();


private:
    MainWindow *mainWindow;
    Level1StartDialog *level1StartDialog;
    Level2StartDialog *level2StartDialog;
    Level3StartDialog *level3StartDialog;

    void showLevelStart(int level);
    void handleGameResult(bool won, int level, int difficulty);
    void returnToMainMenu();    // ★ 统一处理：停计时 + 保存成绩 + 回主菜单
    static void saveLeaderboardEntry(const QString &name, int fragments, int time);   // ★ 加这行
};

#endif // GAMECONTROLLER_H
