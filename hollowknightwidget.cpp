#include "hollowknightwidget.h"
#include <QPainter>
#include <QTime>
#include <QtMath>
#include <cmath>
#include <QPushButton>

HollowKnightWidget::HollowKnightWidget(QWidget *parent, int difficulty, QSize size)
    : QWidget(parent)
    , difficulty(difficulty)
    , playerX(100)
    , playerY(300)
    , keyLeft(false)
    , keyRight(false)
    , keyUp(false)
    , keyDown(false)
    , keyShoot(false)
    , shootCooldown(0)
    , playerHealth(200)
    , playerMaxHealth(200)
    , playerAttack(10)
    , bossX(700)
    , bossY(250)
    , bossHealth(100)
    , bossMaxHealth(100)
    , bossAttack(5)
    , bossMoveTimer(0)
    , bossShootTimer(0)
{
    setFixedSize(size);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    switch (difficulty) {
    case 1:
        playerHealth = 200; playerMaxHealth = 200;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 5;
        break;
    case 2:
        playerHealth = 150; playerMaxHealth = 150;
        bossHealth = 150;  bossMaxHealth = 150;
        bossAttack = 8;
        break;
    case 3:
        playerHealth = 100; playerMaxHealth = 100;
        bossHealth = 200;  bossMaxHealth = 200;
        bossAttack = 10;
        break;
    }

    gameTimer = new QTimer(this);
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateGame()));
    gameTimer->start(50);

    // 返回主菜单按钮
    QPushButton *backBtn = new QPushButton(this);
    backBtn->setGeometry(1150, 0, 50, 30);
    backBtn->setText("返回");
    backBtn->setStyleSheet("QPushButton { background-color: rgba(200,200,200,200); border: 1px solid gray; border-radius: 5px; }"
                           "QPushButton:hover { background-color: rgba(255,255,255,230); }");
    backBtn->setFocusPolicy(Qt::NoFocus);
    connect(backBtn, SIGNAL(clicked()), this, SIGNAL(returnToMenu()));
}

HollowKnightWidget::~HollowKnightWidget() {}

void HollowKnightWidget::setDifficulty(int difficulty)
{
    this->difficulty = difficulty;
    switch (difficulty) {
    case 1:
        playerHealth = 200; playerMaxHealth = 200;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 5;
        break;
    case 2:
        playerHealth = 150; playerMaxHealth = 150;
        bossHealth = 150;  bossMaxHealth = 150;
        bossAttack = 8;
        break;
    case 3:
        playerHealth = 100; playerMaxHealth = 100;
        bossHealth = 200;  bossMaxHealth = 200;
        bossAttack = 10;
        break;
    }
}

void HollowKnightWidget::reset()
{
    playerX = 100; playerY = 300;
    keyLeft = false; keyRight = false; keyUp = false; keyDown = false;
    keyShoot = false; shootCooldown = 0;
    bossX = 700; bossY = 250;
    bossMoveTimer = 0; bossShootTimer = 0;
    playerBullets.clear();
    bossBullets.clear();

    switch (difficulty) {
    case 1:
        playerHealth = 200; playerMaxHealth = 200;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 5;
        break;
    case 2:
        playerHealth = 150; playerMaxHealth = 150;
        bossHealth = 150;  bossMaxHealth = 150;
        bossAttack = 8;
        break;
    case 3:
        playerHealth = 100; playerMaxHealth = 100;
        bossHealth = 200;  bossMaxHealth = 200;
        bossAttack = 10;
        break;
    }
}

void HollowKnightWidget::initGame()
{
    playerBullets.clear();
    bossBullets.clear();
}

bool HollowKnightWidget::checkCollision(const QRect &rect1, const QRect &rect2)
{
    return rect1.intersects(rect2);
}

// ===================== 玩家移动 + 射击 =====================

void HollowKnightWidget::movePlayer()
{
    const int MOVE_SPEED = 5;

    // ★ 移动：追踪按键方向
    if (keyLeft)  playerX -= MOVE_SPEED;
    if (keyRight) playerX += MOVE_SPEED;
    if (keyUp)    playerY -= MOVE_SPEED;
    if (keyDown)  playerY += MOVE_SPEED;

    // 边界
    if (playerX < 0) playerX = 0;
    if (playerX > width() - 30) playerX = width() - 30;
    if (playerY < 0) playerY = 0;
    if (playerY > height() - 30) playerY = height() - 30;

    // ===== ★ 根据按键确定射击方向 =====
    if (keyShoot && shootCooldown <= 0) {
        double vx = 0, vy = 0;
        if (keyLeft)  vx = -1;
        if (keyRight) vx = 1;
        if (keyUp)    vy = -1;
        if (keyDown)  vy = 1;

        // 如果没有按方向键，默认朝右
        if (vx == 0 && vy == 0) { vx = 1; vy = 0; }

        // 归一化（对角线方向速度一致）
        double len = std::sqrt(vx*vx + vy*vy);
        if (len > 0) { vx /= len; vy /= len; }

        spawnPlayerBullet(vx * 10, vy * 10);
        shootCooldown = 12;
    }
    if (shootCooldown > 0) shootCooldown--;

    // ===== 玩家子弹移动 + 碰撞 =====
    for (PlayerBullet &b : playerBullets) {
        if (!b.active) continue;
        b.x += b.vx;
        b.y += b.vy;

        QRect bulletRect((int)b.x, (int)b.y, 12, 12);
        QRect bossRect(bossX, bossY, 50, 50);

        if (checkCollision(bulletRect, bossRect)) {
            b.active = false;
            bossHealth -= playerAttack;
            if (bossHealth <= 0) {
                emit gameWon();
                return;
            }
        }

        if (b.x < -50 || b.x > width() + 50 || b.y < -50 || b.y > height() + 50)
            b.active = false;
    }
}

// ★ 生成玩家子弹
void HollowKnightWidget::spawnPlayerBullet(double vx, double vy)
{
    PlayerBullet b;
    b.x = playerX + 15;
    b.y = playerY + 15;
    b.vx = vx;
    b.vy = vy;
    b.active = true;
    playerBullets.append(b);
}

// ===================== Boss 逻辑 =====================

void HollowKnightWidget::updateBoss()
{
    // Boss 左右移动
    bossMoveTimer++;
    bossX += qSin(bossMoveTimer * 0.03) * 3;  // 正弦左右摆动

    // 边界
    if (bossX < 50) bossX = 50;
    if (bossX > width() - 100) bossX = width() - 100;

    // Boss 上下浮动
    bossY = 250 + qSin(bossMoveTimer * 0.04) * 60;

    // Boss 射击
    bossShootTimer++;
    int shootInterval = (difficulty == 1) ? 50 : (difficulty == 2) ? 40 : 30;
    if (bossShootTimer >= shootInterval) {
        spawnBossBullet();
        bossShootTimer = 0;
    }

    // Boss 子弹移动 + 碰撞
    QRect playerRect(playerX, playerY, 30, 30);
    for (BossBullet &b : bossBullets) {
        if (!b.active) continue;
        b.x += b.vx;
        b.y += b.vy;

        QRect bulletRect((int)b.x, (int)b.y, 15, 15);
        if (checkCollision(playerRect, bulletRect)) {
            b.active = false;
            playerHealth -= bossAttack;
            if (playerHealth <= 0) {
                emit gameLost();
                return;
            }
        }

        if (b.x < -50 || b.x > width() + 50 || b.y < -50 || b.y > height() + 50)
            b.active = false;
    }
}

// ★ Boss 朝玩家方向射击
void HollowKnightWidget::spawnBossBullet()
{
    // 计算朝玩家的方向
    double dx = playerX - bossX;
    double dy = playerY - bossY;
    double len = std::sqrt(dx*dx + dy*dy);
    if (len == 0) return;
    double nx = dx / len;
    double ny = dy / len;

    double speed = 5;
    int bulletCount = (difficulty == 1) ? 1 : (difficulty == 2) ? 2 : 3;

    for (int i = 0; i < bulletCount; i++) {
        BossBullet b;
        b.x = bossX + 25;
        b.y = bossY + 25;

        // 子弹方向：主方向 + 微小的角度扩散
        double angleOffset = (i - (bulletCount - 1) / 2.0) * 0.3;
        double angle = std::atan2(ny, nx) + angleOffset;
        b.vx = std::cos(angle) * speed;
        b.vy = std::sin(angle) * speed;
        b.active = true;
        bossBullets.append(b);
    }
}

// ===================== 绘制 =====================

void HollowKnightWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    // 玩家
    painter.fillRect(playerX, playerY, 30, 30, Qt::blue);

    // Boss
    painter.fillRect(bossX, bossY, 50, 50, Qt::darkRed);

    // 玩家子弹
    for (const PlayerBullet &b : playerBullets) {
        if (b.active)
            painter.fillRect((int)b.x, (int)b.y, 12, 12, Qt::cyan);
    }

    // Boss 子弹
    for (const BossBullet &b : bossBullets) {
        if (b.active)
            painter.fillRect((int)b.x, (int)b.y, 15, 15, Qt::darkYellow);
    }

    // ★ Boss 血条（顶部中间，红色）
    int bossBarWidth = (bossHealth * 300) / bossMaxHealth;
    painter.setPen(Qt::NoPen);
    painter.fillRect(width() / 2 - 150, 20, 300, 25, Qt::darkGray);
    painter.fillRect(width() / 2 - 150, 20, bossBarWidth, 25, Qt::red);
    painter.setPen(Qt::white);
    QFont f;
    f.setPointSize(11);
    f.setBold(true);
    painter.setFont(f);
    painter.drawText(width() / 2 - 140, 37, QString("BOSS: %1/%2").arg(bossHealth).arg(bossMaxHealth));

    // ★ 玩家血条（在 Boss 血条正下方，绿色）
    int playerBarWidth = (playerHealth * 200) / playerMaxHealth;
    painter.setPen(Qt::NoPen);
    painter.fillRect(width() / 2 - 100, 55, 200, 20, Qt::darkGray);
    painter.fillRect(width() / 2 - 100, 55, playerBarWidth, 20, Qt::green);
    painter.setPen(Qt::white);
    painter.setFont(f);
    painter.drawText(width() / 2 - 90, 70, QString("HP: %1/%2").arg(playerHealth).arg(playerMaxHealth));
}

// ===================== 按键 =====================

void HollowKnightWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:        keyUp = true; break;
    case Qt::Key_S:
    case Qt::Key_Down:      keyDown = true; break;
    case Qt::Key_A:
    case Qt::Key_Left:      keyLeft = true; break;
    case Qt::Key_D:
    case Qt::Key_Right:     keyRight = true; break;
    case Qt::Key_J:
    case Qt::Key_Space:     keyShoot = true; break;
    }
}

void HollowKnightWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:        keyUp = false; break;
    case Qt::Key_S:
    case Qt::Key_Down:      keyDown = false; break;
    case Qt::Key_A:
    case Qt::Key_Left:      keyLeft = false; break;
    case Qt::Key_D:
    case Qt::Key_Right:     keyRight = false; break;
    case Qt::Key_J:
    case Qt::Key_Space:     keyShoot = false; break;
    }
}

void HollowKnightWidget::updateGame()
{
    movePlayer();
    updateBoss();
    update();
}
