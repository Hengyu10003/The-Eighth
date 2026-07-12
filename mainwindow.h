#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QTimer>
#include <QSoundEffect>

class GameController;
class MazeWidget;
class LinkGameWidget;
class MarioWidget;
class HollowKnightWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showMaze(int level);
    void showLinkGame(int difficulty);
    void showMarioGame(int difficulty);
    void showHollowKnightGame(int difficulty);

    void startGameTimer();
    void stopGameTimer();
    int getGameTime() const;

    QString getPlayerName() const;
    void setPlayerName(const QString &name);

    int getCurrentLevel() const;
    void setCurrentLevel(int level);
    int getCurrentDifficulty() const;
    void setCurrentDifficulty(int difficulty);

    int getCollectedFragments() const;
    void addFragment();
    void resetProgress();

signals:
    void gameStarted();
    void gameFinished(int fragments, int time);

public slots:
    void showMainMenu();   // 返回主菜单（作为槽，支持信号连接）

private slots:
    void onInputNameClicked();
    void onStartGameClicked();
    void onBackgroundClicked();
    void onLeaderboardClicked();
    void onExitClicked();
    void updateTimer();
    void updateBgFrame();
    void onGamePaused(bool paused);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Ui::MainWindow *ui;

    // 背景动画
    QPixmap bgPixmap[4];
    int bgFrame;
    QTimer *bgAnimTimer;

    QTimer *gameTimer;
    QSoundEffect *bgMusic;
    QSoundEffect *m_buttonSound;
    int elapsedTime;
    QString playerName;

    int currentLevel;
    int currentDifficulty;
    int collectedFragments;

    GameController *gameController;

    MazeWidget *mazeWidget;
    LinkGameWidget *linkGameWidget;
    MarioWidget *marioWidget;
    HollowKnightWidget *hollowKnightWidget;

    QSize gameSize() const;  // 返回背景图大小（所有游戏界面的统一尺寸）
};

#endif // MAINWINDOW_H
