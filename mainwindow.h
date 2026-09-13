/*
头文件保护机制：
作用：防止同一个.h被重复包含两次，导致编译报错
原理：第一次包含时，对其进行定义
     第二次包含时，直接跳到#endif*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//头文件包含====================================================================================================================================
#include <QMainWindow>
/*Qt提供的标准窗口类：自带标题栏、最大/最小/关闭按钮*/
#include <QPixmap>
/*Qt的图片存储类*/
#include <QTimer>
/*Qt的定时器类*/
#include <QSoundEffect>
/*Qt的音效播放类*/

//前置声明=======================================================================================================
class GameController;//中央控制器
class MazeWidget;//迷宫
class LinkGameWidget;//连线
class MarioWidget;//马里奥
class HollowKnightWidget;//BOSS

//ui命名空间前置声明
namespace Ui {
class MainWindow;
}

//MainWindow类定义=============================================================
class MainWindow : public QMainWindow//继承标准窗口，获得标准窗口的所有功能
{
    Q_OBJECT//让signal,slot,QTimer可以正常工作（让信息与槽正常连接，工作）

public:
    //构造函数
    explicit MainWindow(QWidget *parent = 0);
    /*
    作用：初始化所有成员变量、加载背景图片、创建四个游戏Widget、设置信号槽连接、
         加载音效、安装全局事件过滤器
    */
    //析构函数
    ~MainWindow();
    /*程序关闭时清理内存new出来的对象*/

    //被GameController调用，切换到对应游戏界面（参数：等级编号）
    void showMaze(int level);
    void showLinkGame(int difficulty);
    void showMarioGame(int difficulty);
    void showHollowKnightGame(int difficulty);

    //计时器
    void startGameTimer();//开始游戏全局计时器
    void stopGameTimer();//结束游戏全局计时器
    int getGameTime() const;//返回当前已用秒数（完成时间）

    //玩家昵称
    QString getPlayerName() const;//返回玩家昵称
    void setPlayerName(const QString &name);//保存玩家昵称

    //关卡/难度读写
    //记录当前在第几关
    int getCurrentLevel() const;
    void setCurrentLevel(int level);
    //记录当前难度
    int getCurrentDifficulty() const;
    void setCurrentDifficulty(int difficulty);

    //碎片
    int getCollectedFragments() const;//已收集碎片数
    void addFragment();//通关后，碎片数+1
    void resetProgress();//重置本难度碎片数为0（重新开始游戏时调用）
//信号======================================================================================
signals:
    void gameStarted();//开始游戏信号
    void gameFinished(int fragments, int time);//结束游戏信号（参数：碎片数，完成时间）

//槽=========================================================================================
//公有槽----------------------------------------------------------------------------------
public slots:
    void showMainMenu();   // 返回主菜单（作为槽，支持信号连接），gamecontroller控制它
//私有槽------------------------------------------------------------------------------
private slots:
    void onInputNameClicked();
    void onStartGameClicked();
    void onBackgroundClicked();
    void onLeaderboardClicked();
    void onExitClicked();

    void updateTimer();//音乐
    void updateBgFrame();//背景图片

    void onGamePaused(bool paused);//ESC：暂停游戏信号

protected:
    void paintEvent(QPaintEvent *event);//重绘
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::MainWindow *ui;//指向 mainwindow.ui 编译后生成的界面类的指针

    // 背景动画
    QPixmap bgPixmap[4];
    int bgFrame;//当前显示第几张图片
    QTimer *bgAnimTimer;//背景动画计时器

    //游戏计时
    QTimer *gameTimer;//游戏用时定时器
    int elapsedTime;//本局用时

    //音效
    QSoundEffect *bgMusic;//背景音乐播放器
    QSoundEffect *m_buttonSound;//按钮音效播放器

    //玩家昵称数据
    QString playerName;

    //关卡状态
    int currentLevel;//当前关卡数
    int currentDifficulty;//当前难度等级

    //已收集碎片总数
    int collectedFragments;

    //游戏流程控制器
    GameController *gameController;

    //游戏指针对象
    MazeWidget *mazeWidget;
    LinkGameWidget *linkGameWidget;
    MarioWidget *marioWidget;
    HollowKnightWidget *hollowKnightWidget;

    QSize gameSize() const;  // 返回背景图大小（所有游戏界面的统一尺寸）
};

//头文件保护结束
#endif // MAINWINDOW_H
