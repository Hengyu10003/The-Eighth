#ifndef MARIOWIDGET_H
#define MARIOWIDGET_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QRect>
#include <QList>
#include <QPixmap>

#include "config.h"

class MarioWidget : public QWidget
{
    Q_OBJECT

public:
    MarioWidget(QWidget *parent = 0, int difficulty = 1, QSize size = QSize(800, 600));
    ~MarioWidget();

    void reset();

signals:
    void gameWon();
    void gameLost();
    void returnToMenu();
    void gamePaused(bool paused);

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
    int invincibleCount;

    //攻击键用 Qt 事件（需要在释放时立即取消）
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

    struct Obstacle {
        double x; double y; double size; // 正方形
    };
    QList<Obstacle> obstacles;

    QTimer *gameTimer;
    bool m_gameOver;   // 防止重复发射 gameWon/gameLost
    bool m_keyLeft;    // A键/左箭头按下
    bool m_keyRight;   // D键/右箭头按下
    QPixmap m_enemyPixmap;
    QPixmap m_platformPixmap;
    QPixmap m_obstaclePixmap;
    QPixmap m_playerPixmapL[4];   // 向左移动动画4帧
    QPixmap m_playerPixmapR[4];   // 向右移动动画4帧
    QPixmap m_coinPixmap1;
    QPixmap m_coinPixmap2;
    QPixmap m_bgPixmap;
    int m_playerWidth;
    int m_playerHeight;
    int m_coinFrame;
    int m_coinFrameCounter;
    int m_playerFrame;         // 玩家动画帧 0-3
    int m_playerFrameCounter;  // 玩家动画计数器
    bool m_paused;
};

#endif // MARIOWIDGET_H
