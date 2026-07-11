#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QApplication>
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
    bgFrame(0),
    bgAnimTimer(nullptr),
    gameTimer(nullptr),
    elapsedTime(0),
    currentLevel(1),
    currentDifficulty(1),
    collectedFragments(0),
    gameController(nullptr),
    mazeWidget(nullptr),
    linkGameWidget(nullptr),
    marioWidget(nullptr),
    hollowKnightWidget(nullptr),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    gameTimer = new QTimer(this);
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateTimer()));

    gameController = new GameController(this);

    // 加载4张背景图
    bgPixmap[0].load(STARTGAME_BACKGROUND1);
    bgPixmap[1].load(STARTGAME_BACKGROUND2);
    bgPixmap[2].load(STARTGAME_BACKGROUND3);
    bgPixmap[3].load(STARTGAME_BACKGROUND4);

    // 背景动画定时器
    bgAnimTimer = new QTimer(this);
    connect(bgAnimTimer, SIGNAL(timeout()), this, SLOT(updateBgFrame()));
    bgAnimTimer->start(450);

    // 连接 UI 文件中的按钮
    connect(ui->inputNameBtn, SIGNAL(clicked()), this, SLOT(onInputNameClicked()));
    connect(ui->startGameBtn, SIGNAL(clicked()), this, SLOT(onStartGameClicked()));
    connect(ui->backgroundBtn, SIGNAL(clicked()), this, SLOT(onBackgroundClicked()));
    connect(ui->leaderboardBtn, SIGNAL(clicked()), this, SLOT(onLeaderboardClicked()));
    connect(ui->exitBtn, SIGNAL(clicked()), this, SLOT(onExitClicked()));

    showMainMenu();
}

MainWindow::~MainWindow()
{
    delete gameController;
    delete gameTimer;
    delete bgAnimTimer;
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    QPainter painter(this);
    if (!bgPixmap[bgFrame].isNull()) {
        painter.drawPixmap(rect(), bgPixmap[bgFrame]);
    }
}

void MainWindow::updateBgFrame()
{
    bgFrame = (bgFrame + 1) % 4;
    update();
}

QSize MainWindow::gameSize() const
{
    return !bgPixmap[0].isNull() ? bgPixmap[0].size() : QSize(800, 600);
}

void MainWindow::showMainMenu()
{
    // 窗口大小 = 背景图片大小
    if (!bgPixmap[0].isNull()) {
        setFixedSize(bgPixmap[0].size());
    }

    // 移除游戏界面，恢复 UI 设计的 centralwidget
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        old->deleteLater();
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    setCentralWidget(ui->centralwidget);
    ui->centralwidget->show();
    ui->centralwidget->setStyleSheet("");
}

void MainWindow::showMaze(int level)
{
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        old->deleteLater();
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    QSize size = gameSize();
    mazeWidget = new MazeWidget(this, level, size);
    setCentralWidget(mazeWidget);
    setFixedSize(size);

    connect(mazeWidget, SIGNAL(mazeCompleted()), gameController, SLOT(onMazeCompleted()));
    connect(mazeWidget, SIGNAL(returnToMenu()), this, SLOT(showMainMenu()));
}

void MainWindow::showLinkGame(int difficulty)
{
    QWidget *old = takeCentralWidget();
    if (old && old != ui->centralwidget) {
        old->deleteLater();
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    mazeWidget = nullptr;
    marioWidget = nullptr;
    hollowKnightWidget = nullptr;

    linkGameWidget = new LinkGameWidget(this, difficulty, gameSize());
    setCentralWidget(linkGameWidget);
    setFixedSize(gameSize());

    connect(linkGameWidget, SIGNAL(gameWon()), gameController, SLOT(onLinkGameWon()));
    connect(linkGameWidget, SIGNAL(gameLost()), gameController, SLOT(onLinkGameLost()));
    connect(linkGameWidget, SIGNAL(returnToMenu()), this, SLOT(showMainMenu()));

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
        old->deleteLater();
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    marioWidget = new MarioWidget(this, difficulty, gameSize());
    setCentralWidget(marioWidget);
    setFixedSize(gameSize());

    connect(marioWidget, SIGNAL(gameWon()), gameController, SLOT(onMarioGameWon()));
    connect(marioWidget, SIGNAL(gameLost()), gameController, SLOT(onMarioGameLost()));
    connect(marioWidget, SIGNAL(returnToMenu()), this, SLOT(showMainMenu()));

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
        old->deleteLater();
    }
    if (old == ui->centralwidget) {
        old->hide();
    }

    hollowKnightWidget = new HollowKnightWidget(this, difficulty, gameSize());
    setCentralWidget(hollowKnightWidget);
    setFixedSize(gameSize());

    connect(hollowKnightWidget, SIGNAL(gameWon()), gameController, SLOT(onHollowKnightWon()));
    connect(hollowKnightWidget, SIGNAL(gameLost()), gameController, SLOT(onHollowKnightLost()));
    connect(hollowKnightWidget, SIGNAL(returnToMenu()), this, SLOT(showMainMenu()));

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
