#ifndef HOLLOWKNIGHTWIDGET_H
#define HOLLOWKNIGHTWIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QRect>
#include <QList>
#include <QPixmap>

#include "config.h"

class HollowKnightWidget : public QWidget
{
    Q_OBJECT

public:
    HollowKnightWidget(QWidget *parent = 0, int difficulty = 1, QSize size = QSize(1000, 700));
    ~HollowKnightWidget();

    void setDifficulty(int difficulty);
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

private slots:
    void updateGame();

private:
    void initGame();
    bool checkCollision(const QRect &rect1, const QRect &rect2);
    void movePlayer();
    void updateBoss();
    void spawnBossBullet();
    void spawnPlayerBullet(double vx, double vy);

    int difficulty;

    // 玩家
    int playerX;
    int playerY;
    bool keyLeft;
    bool keyRight;
    bool keyUp;
    bool keyDown;
    bool keyShoot;
    int shootCooldown;

    int m_lastDirectionX;
    QPixmap m_playerPixmapL[4];
    QPixmap m_playerPixmapR[4];
    int m_playerFrame;
    int m_playerFrameCounter;

    QPixmap m_bulletPixmap[2];
    int m_bulletFrame;
    QPixmap m_bossBulletPixmap[3];
    QPixmap m_bossPixmap[4];
    int m_bossFrame;
    int m_bossFrameCounter;

    int playerHealth;
    int playerMaxHealth;
    int playerAttack;

    // ★ 玩家子弹（速度带 XY 两个方向）
    struct PlayerBullet {
        double x; double y; double vx; double vy; bool active;
    };
    QList<PlayerBullet> playerBullets;

    // Boss
    int bossX;
    int bossY;
    int bossHealth;
    int bossMaxHealth;
    int bossAttack;
    int bossMoveTimer;

    // ★ Boss 子弹（速度带 XY 两个方向）
    struct BossBullet {
        double x; double y; double vx; double vy; bool active; int tex;
    };
    QList<BossBullet> bossBullets;
    int bossShootTimer;

    QTimer *gameTimer;
    bool m_paused;
};

#endif // HOLLOWKNIGHTWIDGET_H
