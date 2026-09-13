#include "gamecontroller.h"
#include "mainwindow.h"
#include "level1startdialog.h"
#include "level2startdialog.h"
#include "level3startdialog.h"
#include "result1_1dialog.h"
#include "result1_2dialog.h"
#include "result1_3dialog.h"
#include "result2_1dialog.h"
#include "result2_2dialog.h"
#include "result2_3dialog.h"
#include "result3_1dialog.h"
#include "result3_2dialog.h"
#include "result3_3dialog.h"
#include "gameresultdialog.h"
#include "finaldialog.h"
#include "leaderboarddialog.h"

#include <QTimer>//定时器，用来延迟执行
#include <QFile>//读写文件（保存排行榜）
#include <QTextStream>//把文字写入文件or从文件读出
#include <QMessageBox>//系统消息弹窗

//蜘蛛的关节、经脉

//构造函数：mainwindow创建这个控制器时执行
GameController::GameController(MainWindow *mainWindow)
    : QObject(mainWindow)//认父
    , mainWindow(mainWindow)//父亲的指针
    , gameResultDialog(nullptr)//初始时不初始化失败结果弹窗
{
}

//析构函数：清理失败弹窗
GameController::~GameController()
{
    delete gameResultDialog;//删除失败结果弹窗
}

//关卡处理========================================================================================================================================================================================================================================================================================================================================================================
//迷宫通关后
void GameController::onMazeCompleted()
{
    //等待迷宫通关事件结束之后下一个事件
    QTimer::singleShot(0, this, [this]() {
        showLevelStart(mainWindow->getCurrentLevel());
    });
}

//数字连线胜利
void GameController::onLinkGameWon()
{
    mainWindow->addFragment();//加一个碎片
    handleGameResult(true, 1, mainWindow->getCurrentDifficulty());//胜利，第一关，当前难度
}
//数字连线失败
void GameController::onLinkGameLost()
{
    handleGameResult(false, 1, mainWindow->getCurrentDifficulty());
}

//超级马里奥胜利
void GameController::onMarioGameWon()
{
    mainWindow->addFragment();
    handleGameResult(true, 2, mainWindow->getCurrentDifficulty());
}
//超级马里奥失败
void GameController::onMarioGameLost()
{
    handleGameResult(false, 2, mainWindow->getCurrentDifficulty());
}

//空洞骑士胜利
void GameController::onHollowKnightWon()
{
    mainWindow->addFragment();
    handleGameResult(true, 3, mainWindow->getCurrentDifficulty());
}
//空洞骑士失败
void GameController::onHollowKnightLost()
{
    handleGameResult(false, 3, mainWindow->getCurrentDifficulty());
}

//关卡结束选择分支事件处理================================================================================================================================================================================
//打开对应关卡2
void GameController::onLevelStart(int gameType)
{
    Q_UNUSED(gameType);// 抑制"未使用参数"的编译器警告（C++ 编译器发现函数参数声明了但没使用就会警告）
    int level = mainWindow->getCurrentLevel();//当前关卡数
    int difficulty = mainWindow->getCurrentDifficulty();//当前难度

    switch (level) {
    case 1:
        mainWindow->showLinkGame(difficulty);//第一关，打开数字连线
        break;
    case 2:
        mainWindow->showMarioGame(difficulty);//第二关，打开超级马里奥
        break;
    case 3:
        mainWindow->showHollowKnightGame(difficulty);//第三关，打开空洞骑士
        break;
    }
}

//关卡结束按钮选择4
void GameController::onGameResult(int result)
{
    int level = mainWindow->getCurrentLevel();
    int difficulty = mainWindow->getCurrentDifficulty();

    //提高难度
    if (result == 0) {
        if (difficulty < 3) {//还有更高难度
            mainWindow->setCurrentDifficulty(difficulty + 1);//难度+1
            showLevelStart(level);//打开本关卡开始界面
        }
        else {//没有更高难度
            if (level < 3) {//有更高关卡
                mainWindow->setCurrentLevel(level + 1);//进入下一关
                mainWindow->setCurrentDifficulty(1);//难度重置为简单
                mainWindow->showMaze(mainWindow->getCurrentLevel());//重新开始本关卡的迷宫
            } else {//没有更高关卡
                returnToMainMenu();//三关通过——》返回开始游戏界面
            }
        }
    }
    //重新开始
    else if (result == 1) {
        int curLevel = mainWindow->getCurrentLevel();
        int curDifficulty = mainWindow->getCurrentDifficulty();

        if (curLevel == 1)
            mainWindow->showLinkGame(curDifficulty);
        else if (curLevel == 2)
            mainWindow->showMarioGame(curDifficulty);
        else if (curLevel == 3)
            mainWindow->showHollowKnightGame(curDifficulty);
    }
    //返回主菜单
    else if (result == 2) {
        int curLevel = mainWindow->getCurrentLevel();
        if (curLevel == 3) {
            FinalDialog dlg(mainWindow);
            connect(&dlg, &FinalDialog::backToMenu, [this]() {
                mainWindow->stopGameTimer();//停掉计时
                saveLeaderboardEntry(mainWindow->getPlayerName(),mainWindow->getCollectedFragments(),mainWindow->getGameTime());//把成绩存到排行榜
                mainWindow->showMainMenu();//返回主菜单界面
            });
            dlg.exec();
        } else if (curLevel == 2) {
            showLevelStart(curLevel);
        } else {
            returnToMainMenu();
        }

    }
    //进入下一关
    else if (result == 3) {
        int curLevel = mainWindow->getCurrentLevel();
        if (curLevel < 3) {//还有关卡未执行
            mainWindow->setCurrentLevel(curLevel + 1);
            mainWindow->setCurrentDifficulty(1);
            mainWindow->showMaze(mainWindow->getCurrentLevel());
        } else {
            FinalDialog dlg(mainWindow);
            connect(&dlg, &FinalDialog::backToMenu, [this]() {
                mainWindow->stopGameTimer();
                saveLeaderboardEntry(mainWindow->getPlayerName(),mainWindow->getCollectedFragments(),mainWindow->getGameTime());
                mainWindow->showMainMenu();
            });
            dlg.exec();
        }
    }
}

//关卡开始界面选择1
void GameController::showLevelStart(int level)
{
    // 根据关卡使用对应的 UI 设计对话框
    QDialog *dialog = nullptr;
    if (level == 1) {
        dialog = new Level1StartDialog(mainWindow);
    } else if (level == 2) {
        dialog = new Level2StartDialog(mainWindow);
    } else {
        dialog = new Level3StartDialog(mainWindow);
    }

    //dialog按键结果以及关闭dialog
    int result = dialog->exec();
    dialog->deleteLater();

    //结果去向
    if (result == QDialog::Rejected) {
        returnToMainMenu();
    } else {
        onLevelStart(level);
    }
}

//返回主菜单
void GameController::returnToMainMenu()
{
    mainWindow->stopGameTimer();//停止计时
    saveLeaderboardEntry(mainWindow->getPlayerName(),mainWindow->getCollectedFragments(),mainWindow->getGameTime());//保存排行榜
    QTimer::singleShot(0, mainWindow, [this]() {//执行返回主菜单
        mainWindow->showMainMenu();
    });
}

//胜利/失败后弹窗3
void GameController::handleGameResult(bool won, int level, int difficulty)
{
    if (won) {
        // 胜利：根据关卡和难度使用对应 UI 对话框
        QDialog *dialog = nullptr;
        if (level == 1 && difficulty == 1)
            dialog = new Result1_1Dialog(mainWindow);
        else if (level == 1 && difficulty == 2)
            dialog = new Result1_2Dialog(mainWindow);
        else if (level == 1 && difficulty == 3)
            dialog = new Result1_3Dialog(mainWindow);
        else if (level == 2 && difficulty == 1)
            dialog = new Result2_1Dialog(mainWindow);
        else if (level == 2 && difficulty == 2)
            dialog = new Result2_2Dialog(mainWindow);
        else if (level == 2 && difficulty == 3)
            dialog = new Result2_3Dialog(mainWindow);
        else if (level == 3 && difficulty == 1)
            dialog = new Result3_1Dialog(mainWindow);
        else if (level == 3 && difficulty == 2)
            dialog = new Result3_2Dialog(mainWindow);
        else if (level == 3 && difficulty == 3)
            dialog = new Result3_3Dialog(mainWindow);

        if (dialog) {
            connect(dialog, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
            dialog->exec();
            dialog->deleteLater();
        }
    }
    else {
        // 失败：使用通用失败对话框（直接显示按钮，无需确认步骤）
        if (!gameResultDialog) {
            gameResultDialog = new GameResultDialog(mainWindow);
            connect(gameResultDialog, SIGNAL(resultSelected(int)), this, SLOT(onGameResult(int)));
        }
        gameResultDialog->setResult(level, difficulty);
        gameResultDialog->exec();
    }
}

// 保存排行榜记录====================================================================================================================================================================================================================================
void GameController::saveLeaderboardEntry(const QString &name, int fragments, int time)
{
    struct ScoreEntry { QString name; int fragments; int time; };   // 内部结构体：一条排行榜记录
    QList<ScoreEntry> scores;                                       // 存储所有历史记录的列表，int类型

    //=== 第一步：读取已有的排行榜文件 ===
    QFile file("leaderboard.txt");                                  // 排行榜文件（CSV格式）
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {         // 以只读文本模式打开
        QTextStream in(&file);                                      // 创建文本输入流
        while (!in.atEnd()) {                                       // 逐行读取直到文件末尾
            QString line = in.readLine();                           // 读取一行
            QStringList parts = line.split(",");                    // 按逗号分割：昵称,碎片数,用时
            if (parts.size() == 3) {                                // 校验：合法记录必须有3段
                ScoreEntry s;
                s.name = parts[0];                                  // 昵称
                s.fragments = parts[1].toInt();                     // 碎片数（转整型）
                s.time = parts[2].toInt();                          // 用时秒数（转整型）
                scores.append(s);                                   // 加入内存列表//整个函数可读
            }
        }
        file.close();                                               // 关闭文件
    }

    //=== 第二步：更新或新增玩家记录 ===
    bool found = false;
    for (ScoreEntry &s : scores) {                                  // 遍历已有记录
        if (s.name == name) {                                       // 找到同名玩家
            if (fragments > s.fragments || (fragments == s.fragments && time < s.time)) {      // 新纪录碎片更多 或 碎片相同但用时更短
                s.fragments = fragments;                            // 更新碎片数
                s.time = time;                                      // 更新用时
            }
            found = true;                                           // 标记已找到
            break;
        }
    }
    if (!found) {                                                   // 未找到同名玩家
        scores.append({name, fragments, time});                     // 直接追加新记录
    }

    //=== 第三步：排序 ===C++库中本来有的算法（就像迭代器的用法）
    std::sort(scores.begin(), scores.end(), [](const ScoreEntry &a, const ScoreEntry &b) {
        if (a.fragments != b.fragments) return a.fragments > b.fragments;  // 碎片多的排前面
        return a.time < b.time;                                             // 碎片相同，用时少的排前面
    });

    //=== 第四步：写回文件 ===
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {        // 以覆写文本模式打开
        QTextStream out(&file);                                     // 创建文本输出流
        for (const ScoreEntry &s : scores) {                        // 遍历排序后的列表
            out << s.name << "," << s.fragments << "," << s.time << "\n";  // 写入CSV格式
        }
        file.close();                                               // 关闭文件
    }
}
