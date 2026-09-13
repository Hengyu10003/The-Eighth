#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QPainter>
#include <QAbstractButton>
#include "gamecontroller.h"
#include "config.h"
#include "mazewidget.h"
#include "linkgamewidget.h"
#include "mariowidget.h"
#include "hollowknightwidget.h"
#include "inputnamedialog.h"
#include "backgrounddialog.h"
#include "leaderboarddialog.h"

//蜘蛛的肉身

//创建主窗口界面（全局事件初始化设计）====================================================================================================
MainWindow::MainWindow(QWidget *parent) :
    //父类指针
    QMainWindow(parent),
    //背景
    bgFrame(0),//背景帧动画当前索引
    bgAnimTimer(nullptr),//背景动画计时器
    //游戏评价指标
    gameTimer(nullptr),//全局计时器（本身）
    elapsedTime(0),//游戏用时（计时结果）
    currentLevel(1),//当前关卡编号
    currentDifficulty(1),//当前难度
    collectedFragments(0),//收集碎片数量
    //流程控制
    gameController(nullptr),
    //关卡
    mazeWidget(nullptr),
    linkGameWidget(nullptr),
    marioWidget(nullptr),
    hollowKnightWidget(nullptr),
    ui(new Ui::MainWindow)
{
//美工界面设计
    /*图形界面*/
    //ui界面
    ui->setupUi(this);//设置ui界面，加载mainwindow.ui的界面布局（直接在mainwindow界面上进行绘制）
    //背景图
    bgPixmap[0].load(STARTGAME_BACKGROUND1);
    bgPixmap[1].load(STARTGAME_BACKGROUND2);
    bgPixmap[2].load(STARTGAME_BACKGROUND3);
    bgPixmap[3].load(STARTGAME_BACKGROUND4);
        //背景动画定时器
    bgAnimTimer = new QTimer(this);//背景动画定时器
    connect(bgAnimTimer, SIGNAL(timeout()), this, SLOT(updateBgFrame()));//背景动画时间更新
    bgAnimTimer->start(450);//每450ms/0.45s发射一次timeout信号
    /*音频*/
    // 背景音乐
    bgMusic = new QSoundEffect(this);//创建背景音乐播放器，只支持wav
    bgMusic->setSource(QUrl("qrc" BACK_MUSIC));//设置背景音乐的文件路径
    bgMusic->setLoopCount(QSoundEffect::Infinite);//设置循环次数：无限次
    bgMusic->setVolume(0.5f);//设置音量
    bgMusic->play();//开始播放
    // 按钮音效
    m_buttonSound = new QSoundEffect(this);//创建按钮音效播放器
    m_buttonSound->setSource(QUrl("qrc" BUTTON));//设置按钮音效路径
    m_buttonSound->setVolume(0.7f);//设置音效

//存储数据初始化
    //全局计时器
    gameTimer = new QTimer(this);//创建计时项目
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateTimer()));//过了timeout()一定的时间就会执行updateTimer()
    //全局事件过滤：监听所有按钮点击
    qApp->installEventFilter(this);

//连接 UI 文件中的按钮（按钮，信号，本类，槽函数）
    connect(ui->inputNameBtn, SIGNAL(clicked()), this, SLOT(onInputNameClicked()));
    connect(ui->startGameBtn, SIGNAL(clicked()), this, SLOT(onStartGameClicked()));
    connect(ui->backgroundBtn, SIGNAL(clicked()), this, SLOT(onBackgroundClicked()));
    connect(ui->leaderboardBtn, SIGNAL(clicked()), this, SLOT(onLeaderboardClicked()));
    connect(ui->exitBtn, SIGNAL(clicked()), this, SLOT(onExitClicked()));

//游戏流程控制器
    gameController = new GameController(this);//开辟

    showMainMenu();//显示主菜单界面
}
//析构函数
MainWindow::~MainWindow()
{
    delete gameController;   // 删除游戏流程控制器，清理其内部创建的失败结果弹窗
    delete gameTimer;        // 删除游戏计时器，停止计时
    delete bgAnimTimer;      // 删除背景动画定时器，停止帧动画循环
    delete ui;               // 删除由Qt Designer生成的界面布局对象，释放按钮/标签等控件内存
}

//美工========================================================================================================================================================================================================
//把当前帧背景图画到窗口上（重写Qt自己的函数）
void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);              // 父类先画
    QPainter painter(this);                      // 创建画笔，画布是this
    if (!bgPixmap[bgFrame].isNull()) {           // 图片加载成功才画
        painter.drawPixmap(rect(), bgPixmap[bgFrame]); //（绘制范围：整个窗口矩形，要画的图片）
    }
}

//背景帧切换
void MainWindow::updateBgFrame()
{
    bgFrame = (bgFrame + 1) % 4;
    update();
}

//统一游戏界面尺寸
QSize MainWindow::gameSize() const
{
    return !bgPixmap[0].isNull() ? bgPixmap[0].size() : QSize(800, 600);//第一张图：统一尺寸
}

//显示界面*5========================================================================================================================================================================================================================================================================================================================================================================
//显示主窗口界面
void MainWindow::showMainMenu()
{
// 窗口大小 = 背景图片大小
    if (!bgPixmap[0].isNull()) {
        setFixedSize(bgPixmap[0].size());//锁定窗口大小
    }

//移除当前游戏界面，换回开始游戏界面
    QWidget *old = takeCentralWidget();//取出（移除）当前显示的界面
    if (old && old != ui->centralwidget) {//如果不是开始游戏界面
        old->deleteLater();//删掉
    }
    if (old == ui->centralwidget) {//如果已经是主菜单界面
        old->hide();//直接隐藏
    }

    setCentralWidget(ui->centralwidget);    //把主菜单设为中心界面
    ui->centralwidget->show();              //显示主菜单界面
    ui->centralwidget->setStyleSheet("");   //清除可能残留的样式
}

//显示迷宫界面
void MainWindow::showMaze(int level)
{
    //移除当前界面
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        old->deleteLater();
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    //创捷迷宫界面（设为窗口中心
    mazeWidget = new MazeWidget(this, level, gameSize());//新建迷宫窗口
    setCentralWidget(mazeWidget);//设为中央
    setFixedSize(gameSize());//固定窗口大小

    //连接信号
    connect(mazeWidget, SIGNAL(mazeCompleted()), gameController, SLOT(onMazeCompleted()));//迷宫通关
    connect(mazeWidget, SIGNAL(returnToMenu()), this, SLOT(showMainMenu()));//返回菜单
    connect(mazeWidget, SIGNAL(gamePaused(bool)), this, SLOT(onGamePaused(bool)));//暂停
}

//显示数字连线界面
void MainWindow::showLinkGame(int difficulty)
{
    //移除当前界面
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        old->deleteLater();
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    //清楚其他游戏的指针，避免混淆
    mazeWidget = nullptr;
    marioWidget = nullptr;
    hollowKnightWidget = nullptr;

    //创建连线界面
    linkGameWidget = new LinkGameWidget(this, difficulty, gameSize());
    setCentralWidget(linkGameWidget);
    setFixedSize(gameSize());

    //连接信号
    connect(linkGameWidget, SIGNAL(gameWon()), gameController, SLOT(onLinkGameWon()));//胜利
    connect(linkGameWidget, SIGNAL(gameLost()), gameController, SLOT(onLinkGameLost()));//失败
    connect(linkGameWidget, SIGNAL(returnToMenu()), this, SLOT(showMainMenu()));//返回
    connect(linkGameWidget, SIGNAL(gamePaused(bool)), this, SLOT(onGamePaused(bool)));//暂停

    // 确保键盘焦点落在数字连线界面上，使 WASD 和鼠标点击能立即响应
    this->activateWindow();                        // 激活主窗口为当前活动窗口（按键只对这个窗口有效）
    linkGameWidget->setFocus();                    // 将键盘焦点设置到连线游戏界面
    linkGameWidget->activateWindow();              // 激活连线子窗口以增强焦点效果（保险子）
        //重复激活焦点（保险子*2）
    QTimer::singleShot(50, this, [this]() {        // 延迟50ms重试一次（Qt焦点切换有时滞后）
        this->activateWindow();                    // 再次激活主窗口
        if (linkGameWidget) linkGameWidget->setFocus(); // 再次确保连线界面获得焦点
    });
}

//显示超级马里奥界面
void MainWindow::showMarioGame(int difficulty)
{
    //移除当前界面
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        old->deleteLater();
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    //创建超级马里奥界面
    marioWidget = new MarioWidget(this, difficulty, gameSize());
    setCentralWidget(marioWidget);
    setFixedSize(gameSize());

    //连接信号
    connect(marioWidget, SIGNAL(gameWon()), gameController, SLOT(onMarioGameWon()));//胜利
    connect(marioWidget, SIGNAL(gameLost()), gameController, SLOT(onMarioGameLost()));//失败
    connect(marioWidget, SIGNAL(returnToMenu()), this, SLOT(showMainMenu()));//返回
    connect(marioWidget, SIGNAL(gamePaused(bool)), this, SLOT(onGamePaused(bool)));//暂停

    //确保键盘焦点在界面上
    this->activateWindow();              // 将 MainWindow 窗口激活为当前活动窗口（标题栏高亮）
    marioWidget->setFocus();             // 将键盘焦点设置到马里奥游戏界面上，确保按键能直接响应
    marioWidget->activateWindow();       // 将马里奥子窗口也激活（针对内嵌QWidget的焦点提升）
    QTimer::singleShot(50, this, [this]() {  // 延迟50ms再执行一次，因为Qt有时焦点切换不会立即生效
        this->activateWindow();              // 再次激活主窗口
        if (marioWidget) marioWidget->setFocus(); // 再次确保马里奥界面获得焦点
    });
}

//显示空洞骑士游戏界面
void MainWindow::showHollowKnightGame(int difficulty)
{
    //移除当前界面
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        old->deleteLater();
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    //创建空洞骑士界面
    hollowKnightWidget = new HollowKnightWidget(this, difficulty, gameSize());
    setCentralWidget(hollowKnightWidget);
    setFixedSize(gameSize());

    //连接信号
    connect(hollowKnightWidget, SIGNAL(gameWon()), gameController, SLOT(onHollowKnightWon()));//胜利
    connect(hollowKnightWidget, SIGNAL(gameLost()), gameController, SLOT(onHollowKnightLost()));//失败
    connect(hollowKnightWidget, SIGNAL(returnToMenu()), this, SLOT(showMainMenu()));//返回
    connect(hollowKnightWidget, SIGNAL(gamePaused(bool)), this, SLOT(onGamePaused(bool)));//暂停

    // 确保键盘焦点落在 Boss 战界面上，使 WASD 和射击键能立即响应
    this->activateWindow();                              // 激活主窗口为当前活动窗口
    hollowKnightWidget->setFocus();                      // 将键盘焦点设置到 Boss 战界面
    hollowKnightWidget->activateWindow();                // 激活 Boss 战子窗口以增强焦点效果
    QTimer::singleShot(50, this, [this]() {              // 延迟50ms重试一次（Qt焦点切换有时滞后）
        this->activateWindow();                          // 再次激活主窗口
        if (hollowKnightWidget) hollowKnightWidget->setFocus(); // 再次确保 Boss 战界面获得焦点
    });
}

//全局get========================================================================================================================================================================================================================================================================================================================================================================
//时间-----------------------------------------------------------------------------------------------------------------
    //开始计时：记录游戏时长
void MainWindow::startGameTimer()
{
    elapsedTime = 0;//初始化全局计时器
    gameTimer->start(1000);//每隔一秒触发一次timeout信号
}
    //停止计时
void MainWindow::stopGameTimer()
{
    gameTimer->stop();//时钟停止发射timeout信号
}
    //暂停
void MainWindow::onGamePaused(bool paused)
{
    if (paused)
        stopGameTimer();//暂停：计时停
    else
        startGameTimer();//继续：计时重开
}
    //获取完成时间
int MainWindow::getGameTime() const
{
    return elapsedTime;//全局静态
}

//昵称-----------------------------------------------------------------------------------------------------------
    //获取玩家名字
QString MainWindow::getPlayerName() const
{
    return playerName;
}

//关卡/难度/碎片数----------------------------------------------------------------------------------------------------
//关卡
    //获取当前关卡
int MainWindow::getCurrentLevel() const
{
    return currentLevel;
}
    //设置当前关卡
void MainWindow::setCurrentLevel(int level)
{
    currentLevel = level;
}
//难度
    //获取当前难度
int MainWindow::getCurrentDifficulty() const
{
    return currentDifficulty;
}
    //设置当前难度
void MainWindow::setCurrentDifficulty(int difficulty)
{
    currentDifficulty = difficulty;
}
//碎片数
    //获取收集碎片数量
int MainWindow::getCollectedFragments() const
{
    return collectedFragments;
}
    //增加收集碎片数量
void MainWindow::addFragment()
{
    collectedFragments++;
}

//初始化========================================================================================================================================================================================================================================================================================================================================================================
//初始化设置
void MainWindow::resetProgress()
{
    currentLevel = 1;       //关卡设置为1
    currentDifficulty = 1;  //难度设置为简单
    collectedFragments = 0; //碎片设置为0
    elapsedTime = 0;        //全局用时设置为0
}

//计时器执行
void MainWindow::updateTimer()
{
    elapsedTime++;
}

//按钮（跳转事件）========================================================================================================================================================================================================================================================================================================================================================================
//输入名字界面
void MainWindow::onInputNameClicked()
{
    InputNameDialog dialog(this);               //创建输入名字弹窗
    if (dialog.exec() == QDialog::Accepted) {   //如果玩家点击了“确定”
        playerName = dialog.getPlayerName();    //保存玩家名字
    }
}

//开始游戏
void MainWindow::onStartGameClicked()
{
    if (playerName.isEmpty()) {//没有输入名字
        onInputNameClicked();
        if (playerName.isEmpty()) {
            return;//如果还是没输入名字，就不执行初始化代码，知道输入名字为止
        }
    }

    resetProgress();//重置游戏进度
    startGameTimer();//开始计时
    showMaze(currentLevel);//进入第1关
}

//背景介绍
void MainWindow::onBackgroundClicked()
{
    BackgroundDialog dialog(this);
    dialog.exec();//把弹窗显示在屏幕上
}

//排行榜
void MainWindow::onLeaderboardClicked()
{
    LeaderboardDialog dialog(this);
    dialog.exec();
}

//退出游戏
void MainWindow::onExitClicked()
{
    qApp->exit();//退出整个应用程序
}


//全局按钮音效事件过滤器
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    //如果鼠标按下的对象是一个按钮
    if (event->type() == QEvent::MouseButtonPress) {
        if (qobject_cast<QAbstractButton*>(obj)) {//检测点击的事件是什么东西
            m_buttonSound->play();//播放按钮音效
        }
    }
    return QMainWindow::eventFilter(obj, event);//把事件传给按钮本身
}
