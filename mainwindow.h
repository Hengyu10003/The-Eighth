#ifndef MAINWINDOW_H              // 防止头文件被重复包含的宏开关
#define MAINWINDOW_H              // 定义宏 MAINWINDOW_H

#include <QMainWindow>            // QMainWindow 是主窗口类，提供菜单栏、状态栏、中央窗口等功能
#include <QPushButton>            // QPushButton 是按钮控件，用于开始游戏、退出等按钮
#include <QLabel>                 // QLabel 是标签控件，用于显示文本（如"当前玩家：xxx"）
#include <QGridLayout>            // QGridLayout 是网格布局，把按钮排列成行列整齐的界面
#include <QTimer>                 // QTimer 是定时器，用于每秒更新游戏计时
#include <QWidget>                // QWidget 是所有 UI 控件的基类
#include <QPixmap>                // QPixmap 用于加载和绘制图片

class GameController;
class MazeWidget;
class LinkGameWidget;
class MarioWidget;
class HollowKnightWidget;

namespace Ui {
class MainWindow;
}

// ===================== 主窗口类定义 =====================
class MainWindow : public QMainWindow  // 继承自 QMainWindow，这就是整个游戏的主窗口
{
    Q_OBJECT                      // Qt 元对象宏，使这个类支持信号/槽机制

public:
    explicit MainWindow(QWidget *parent = 0);  // 构造函数，explicit 防止隐式类型转换
    ~MainWindow();                              // 析构函数

    // ===== 界面切换函数 =====
    void showMainMenu();            // 显示主菜单（输入昵称、开始游戏、排行榜等按钮）
    void showMaze(int level);       // 显示迷宫（参数 level 表示第几关的迷宫：1/2/3）
    void showLinkGame(int difficulty);          // 显示数字连连看（参数 difficulty：1简单/2中等/3困难）
    void showMarioGame(int difficulty);         // 显示超级马里奥
    void showHollowKnightGame(int difficulty);  // 显示空洞骑士 Boss 战

    // ===== 计时器 =====
    void startGameTimer();          // 开始计时（点击"开始游戏"时调用）
    void stopGameTimer();           // 停止计时（回到主菜单或游戏结束时调用）
    int getGameTime() const;        // 获取当前已用时间（秒）

    // ===== 玩家昵称 =====
    QString getPlayerName() const;  // 获取玩家昵称
    void setPlayerName(const QString &name);  // 设置玩家昵称

    // ===== 关卡与难度 =====
    int getCurrentLevel() const;        // 获取当前关卡（1/2/3）
    void setCurrentLevel(int level);    // 设置当前关卡
    int getCurrentDifficulty() const;   // 获取当前难度（1/2/3）
    void setCurrentDifficulty(int difficulty);  // 设置当前难度

    // ===== 碎片 =====
    int getCollectedFragments() const;  // 获取已收集的碎片数量
    void addFragment();                 // 增加一个碎片（通过一个难度后调用）
    void resetProgress();               // 重置所有进度（新一局游戏开始时）

signals:
    void gameStarted();             // 信号：游戏开始了（目前未使用，预留）
    void gameFinished(int fragments, int time);  // 信号：游戏结束了（目前未使用，预留）

private slots:
    void onInputNameClicked();      // 点击"输入昵称"按钮 → 弹出输入框
    void onStartGameClicked();      // 点击"开始游戏"按钮 → 进入迷宫
    void onBackgroundClicked();     // 点击"背景简介"按钮 → 弹出背景介绍窗口
    void onLeaderboardClicked();    // 点击"排行榜"按钮 → 弹出排行榜窗口
    void onExitClicked();           // 点击"退出游戏"按钮 → 关闭程序
    void updateTimer();             // 定时器回调：每秒 elapsedTime++
    void updateBgFrame();           // 背景动画帧更新

private:
    // ===== UI 文件生成的界面 =====
    Ui::MainWindow *ui;             // 由 mainwindow.ui 生成

    // ===== 背景动画 =====
    QPixmap bgPixmap[4];            // 4张背景图
    int bgFrame;                    // 当前帧索引
    QTimer *bgAnimTimer;            // 背景动画定时器

    QLabel *playerNameLabel;        // 显示"当前玩家：xxx"的标签
    QString playerName;             // 存储当前玩家的昵称

    // ===== 计时 =====
    QTimer *gameTimer;              // 定时器，每秒触发一次 updateTimer
    int elapsedTime;                // 已经过的时间（秒）

    // ===== 游戏状态 =====
    int currentLevel;               // 当前关卡编号（1/2/3）
    int currentDifficulty;          // 当前难度等级（1=简单/2=中等/3=困难）
    int collectedFragments;         // 已收集的碎片数量

    // ===== 核心控制器 =====
    GameController *gameController; // 游戏控制器，负责处理迷宫通关→进入游戏→游戏结果等跳转逻辑

    // ===== 各个小游戏的界面（当前显示哪个，哪个就不为 nullptr）=====
    MazeWidget *mazeWidget;              // 迷宫游戏界面
    LinkGameWidget *linkGameWidget;      // 数字连连看界面
    MarioWidget *marioWidget;            // 超级马里奥界面
    HollowKnightWidget *hollowKnightWidget;  // 空洞骑士 Boss 战界面
};

#endif // MAINWINDOW_H              // 结束头文件保护宏
