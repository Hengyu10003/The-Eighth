#ifndef MARIOWIDGET_H
#define MARIOWIDGET_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QRect>
#include <QList>

#include "config.h"

class MarioWidget : public QWidget
{
    Q_OBJECT

public:
    MarioWidget(QWidget *parent = 0, int difficulty = 1);
    ~MarioWidget();

    void setDifficulty(int difficulty);
    void reset();

signals:
    void gameWon();
    void gameLost();

protected:
    void paintEvent(QPaintEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void updateGame();

private:
    void initLevel();
    bool checkCollision(const QRect &rect1, const QRect &rect2);
    void movePlayer();

    int difficulty;

    double playerWorldX;
    double playerY;
    double playerVelocityX;
    double playerVelocityY;
    bool isOnGround;
    int jumpCount;
    int invincibleCount;    // ★ 金身护体：还剩几次免死

    // ★ 攻击键用 Qt 事件（需要在释放时立即取消）
    bool keyAttack;

    int lastDirection;          // 最后移动方向：1=右，-1=左
    int facingDirection;        // 当前朝向：用于攻击方向

    int score;
    int coins;
    int enemiesKilled;

    double cameraX;
    static const int PLAYER_SCREEN_X = 200;
    static const int WORLD_WIDTH = 4000;
    static const int GROUND_Y = 550;

    struct Coin {
        double x; double y; bool collected;
    };
    QList<Coin> coinList;

    struct Enemy {
        double x; double y; double velocityX;
        int health; bool alive;
    };
    QList<Enemy> enemies;

    struct Platform {
        double x; double y; double width; double height;
    };
    QList<Platform> platforms;

    QTimer *gameTimer;
};

#endif // MARIOWIDGET_H
