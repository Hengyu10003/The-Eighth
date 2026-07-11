#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
#include <QPixmap>
#include <QPainter>
#include "gamecontroller.h"
#include "config.h"
#include "mazewidget.h"
#include "linkgamewidget.h"
#include "mariowidget.h"
#include "hollowknightwidget.h"
#include "inputnamedialog.h"
#include "backgrounddialog.h"
#include "leaderboarddialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    playerName(""),
    elapsedTime(0),
    currentLevel(1),
    currentDifficulty(1),
    collectedFragments(0),
    gameController(nullptr),
    mazeWidget(nullptr),
    linkGameWidget(nullptr),
    marioWidget(nullptr),
    hollowKnightWidget(nullptr),
    playerNameLabel(nullptr),
    gameTimer(nullptr),
    bgFrame(0),
    bgAnimTimer(nullptr),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    gameTimer = new QTimer(this);
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateTimer()));

    gameController = new GameController(this);

    bgPixmap[0].load(STARTGAME_BACKGROUND1);
    bgPixmap[1].load(STARTGAME_BACKGROUND2);
    bgPixmap[2].load(STARTGAME_BACKGROUND3);
    bgPixmap[3].load(STARTGAME_BACKGROUND4);

    bgAnimTimer = new QTimer(this);
    connect(bgAnimTimer, SIGNAL(timeout()), this, SLOT(updateBgFrame()));
    bgAnimTimer->start(450);

    // 连接 UI 文件中的按钮信号
    connect(ui->inputNameBtn, SIGNAL(clicked()), this, SLOT(onInputNameClicked()));
    connect(ui->startGameBtn, SIGNAL(clicked()), this, SLOT(onStartGameClicked()));
    connect(ui->backgroundBtn, SIGNAL(clicked()), this, SLOT(onBackgroundClicked()));
    connect(ui->leaderboardBtn, SIGNAL(clicked()), this, SLOT(onLeaderboardClicked()));
    connect(ui->exitBtn, SIGNAL(clicked()), this, SLOT(onExitClicked()));

    // 添加玩家名标签到布局
    playerNameLabel = new QLabel("", ui->centralwidget);
    playerNameLabel->setAlignment(Qt::AlignCenter);
    playerNameLabel->setStyleSheet("QLabel { color: white; font-size: 18px; font-weight: bold; }");
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    if (layout) {
        layout->insertWidget(0, playerNameLabel);
    }

    showMainMenu();
}

MainWindow::~MainWindow()
{
    delete gameController;
    delete gameTimer;
    delete bgAnimTimer;
    delete ui;
}

void MainWindow::updateBgFrame()
{
    bgFrame = (bgFrame + 1) % 4;
    if (!bgPixmap[bgFrame].isNull()) {
        QPalette palette = ui->centralwidget->palette();
        palette.setBrush(QPalette::Background, bgPixmap[bgFrame]);
        ui->centralwidget->setPalette(palette);
        ui->centralwidget->update();
    }
}

void MainWindow::showMainMenu()
{
    // 窗口大小适应背景图片
    if (!bgPixmap[0].isNull()) {
        setFixedSize(bgPixmap[0].size());
    }

    // 移除当前的中心控件（游戏界面），但不删除 ui->centralwidget
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        delete old;
    }

    // 恢复 ui->centralwidget 作为中心控件
    setCentralWidget(ui->centralwidget);
    ui->centralwidget->show();

    // 设置背景动画
    QPalette palette = ui->centralwidget->palette();
    if (!bgPixmap[0].isNull()) {
        palette.setBrush(QPalette::Background, bgPixmap[bgFrame]);
    } else {
        palette.setColor(QPalette::Background, QColor(30, 30, 30));
    }
    ui->centralwidget->setPalette(palette);
    ui->centralwidget->setAutoFillBackground(true);

    if (!playerName.isEmpty()) {
        playerNameLabel->setText("当前玩家：" + playerName);
    }
}

void MainWindow::showMaze(int level)
{
    // 移除当前中心控件，保护 ui->centralwidget 不被删除
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        delete old;
    }
    // 如果 old 是 ui->centralwidget，只隐藏不删除
    if (old == ui->centralwidget) {
        old->hide();
    }

    mazeWidget = new MazeWidget(this, level);
    setCentralWidget(mazeWidget);
    resize(mazeWidget->size());

    connect(mazeWidget, SIGNAL(mazeCompleted()), gameController, SLOT(onMazeCompleted()));
}

void MainWindow::showLinkGame(int difficulty)
{
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        delete old;
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    mazeWidget = nullptr;
    marioWidget = nullptr;
    hollowKnightWidget = nullptr;

    linkGameWidget = new LinkGameWidget(this, difficulty);
    setCentralWidget(linkGameWidget);
    resize(linkGameWidget->size());

    connect(linkGameWidget, SIGNAL(gameWon()), gameController, SLOT(onLinkGameWon()));
    connect(linkGameWidget, SIGNAL(gameLost()), gameController, SLOT(onLinkGameLost()));

    this->activateWindow();
    linkGameWidget->setFocus();
    linkGameWidget->activateWindow();
    QTimer::singleShot(50, this, [this]() {
        this->activateWindow();
        if (linkGameWidget) linkGameWidget->setFocus();
    });
}

void MainWindow::showMarioGame(int difficulty)
{
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        delete old;
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    marioWidget = new MarioWidget(this, difficulty);
    setCentralWidget(marioWidget);
    resize(marioWidget->size());

    connect(marioWidget, SIGNAL(gameWon()), gameController, SLOT(onMarioGameWon()));
    connect(marioWidget, SIGNAL(gameLost()), gameController, SLOT(onMarioGameLost()));

    this->activateWindow();
    marioWidget->setFocus();
    marioWidget->activateWindow();
    QTimer::singleShot(50, this, [this]() {
        this->activateWindow();
        if (marioWidget) marioWidget->setFocus();
    });
}

void MainWindow::showHollowKnightGame(int difficulty)
{
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        delete old;
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    hollowKnightWidget = new HollowKnightWidget(this, difficulty);
    setCentralWidget(hollowKnightWidget);
    resize(hollowKnightWidget->size());

    connect(hollowKnightWidget, SIGNAL(gameWon()), gameController, SLOT(onHollowKnightWon()));
    connect(hollowKnightWidget, SIGNAL(gameLost()), gameController, SLOT(onHollowKnightLost()));

    this->activateWindow();
    hollowKnightWidget->setFocus();
    hollowKnightWidget->activateWindow();
    QTimer::singleShot(50, this, [this]() {
        this->activateWindow();
        if (hollowKnightWidget) hollowKnightWidget->setFocus();
    });
}

void MainWindow::startGameTimer()
{
    elapsedTime = 0;
    gameTimer->start(1000);
}

void MainWindow::stopGameTimer()
{
    gameTimer->stop();
}

int MainWindow::getGameTime() const
{
    return elapsedTime;
}

QString MainWindow::getPlayerName() const
{
    return playerName;
}

void MainWindow::setPlayerName(const QString &name)
{
    playerName = name;
}

int MainWindow::getCurrentLevel() const
{
    return currentLevel;
}

void MainWindow::setCurrentLevel(int level)
{
    currentLevel = level;
}

int MainWindow::getCurrentDifficulty() const
{
    return currentDifficulty;
}

void MainWindow::setCurrentDifficulty(int difficulty)
{
    currentDifficulty = difficulty;
}

int MainWindow::getCollectedFragments() const
{
    return collectedFragments;
}

void MainWindow::addFragment()
{
    collectedFragments++;
}

void MainWindow::resetProgress()
{
    currentLevel = 1;
    currentDifficulty = 1;
    collectedFragments = 0;
    elapsedTime = 0;
}

void MainWindow::updateTimer()
{
    elapsedTime++;
}

void MainWindow::onInputNameClicked()
{
    InputNameDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        playerName = dialog.getPlayerName();
        if (playerNameLabel && !playerName.isEmpty()) {
            playerNameLabel->setText("当前玩家：" + playerName);
        }
    }
}

void MainWindow::onStartGameClicked()
{
    if (playerName.isEmpty()) {
        onInputNameClicked();
        if (playerName.isEmpty()) {
            return;
        }
    }

    resetProgress();
    startGameTimer();
    showMaze(currentLevel);
}

void MainWindow::onBackgroundClicked()
{
    BackgroundDialog dialog(this);
    dialog.exec();
}

void MainWindow::onLeaderboardClicked()
{
    LeaderboardDialog dialog(this);
    dialog.exec();
}

void MainWindow::onExitClicked()
{
    qApp->exit();
}
