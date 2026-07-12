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
    , m_lastDirectionX(1)
    , m_playerFrame(0)
    , m_playerFrameCounter(0)
    , m_bulletFrame(0)
    , m_bossFrame(0)
    , m_bossFrameCounter(0)
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
    , m_paused(false)
{
    setFixedSize(size);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    m_playerPixmapL[0].load(TUAN1_l);
    m_playerPixmapL[1].load(TUAN2_l);
    m_playerPixmapL[2].load(TUAN3_l);
    m_playerPixmapL[3].load(TUAN4_l);
    m_playerPixmapR[0].load(TUAN1_r);
    m_playerPixmapR[1].load(TUAN2_r);
    m_playerPixmapR[2].load(TUAN3_r);
    m_playerPixmapR[3].load(TUAN4_r);

    m_bulletPixmap[0].load(TAN_BI1);
    m_bulletPixmap[1].load(TAN_BI2);

    m_bossBulletPixmap[0].load(BOSS_BI1);
    m_bossBulletPixmap[1].load(BOSS_BI2);
    m_bossBulletPixmap[2].load(BOSS_BI3);

    m_bossPixmap[0].load(BOSS1);
    m_bossPixmap[1].load(BOSS2);
    m_bossPixmap[2].load(BOSS3);
    m_bossPixmap[3].load(BOSS4);

    switch (difficulty) {
    case 1:
        playerHealth = 200; playerMaxHealth = 200;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 5;
        break;
    case 2:
        playerHealth = 250; playerMaxHealth = 250;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 4;
        break;
    case 3:
        playerHealth = 300; playerMaxHealth = 300;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 3;
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
        playerHealth = 250; playerMaxHealth = 250;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 4;
        break;
    case 3:
        playerHealth = 300; playerMaxHealth = 300;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 3;
        break;
    }
}

void HollowKnightWidget::reset()
{
    playerX = 100; playerY = 300;
    keyLeft = false; keyRight = false; keyUp = false; keyDown = false;
    keyShoot = false; shootCooldown = 0;
    m_lastDirectionX = 1;
    m_playerFrame = 0;
    m_playerFrameCounter = 0;
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
        playerHealth = 250; playerMaxHealth = 250;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 4;
        break;
    case 3:
        playerHealth = 300; playerMaxHealth = 300;
        bossHealth = 100;  bossMaxHealth = 100;
        bossAttack = 3;
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
    if (playerX > width() - 60) playerX = width() - 60;
    if (playerY < 0) playerY = 0;
    if (playerY > height() - 60) playerY = height() - 60;

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

        QRect bulletRect((int)b.x, (int)b.y, 30, 30);
        QRect bossRect(bossX, bossY, 110, 110);

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
    b.x = playerX + 30;
    b.y = playerY + 30;
    b.vx = vx;
    b.vy = vy;
    b.active = true;
    playerBullets.append(b);
}

// ===================== Boss 逻辑 =====================

void HollowKnightWidget::updateBoss()
{
    double speedMult = (difficulty == 1) ? 1.0 : (difficulty == 2) ? 0.9 : 0.8;

    // Boss 左右移动
    bossMoveTimer++;
    bossX += qSin(bossMoveTimer * 0.03) * 3 * speedMult;

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
    QRect playerRect(playerX, playerY, 60, 60);
    for (BossBullet &b : bossBullets) {
        if (!b.active) continue;
        b.x += b.vx;
        b.y += b.vy;

        QRect bulletRect((int)b.x, (int)b.y, 18, 18);
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
        b.tex = qrand() % 3;
        bossBullets.append(b);
    }
}

// ===================== 绘制 =====================

void HollowKnightWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);

    // 玩家：根据朝向选择动画帧
    QPixmap currentFrame;
    if (m_lastDirectionX == -1) {
        currentFrame = m_playerPixmapL[m_playerFrame];
    } else {
        currentFrame = m_playerPixmapR[m_playerFrame];
    }
    if (!currentFrame.isNull()) {
        painter.drawPixmap(playerX, playerY, 60, 60, currentFrame);
    } else {
        painter.fillRect(playerX, playerY, 60, 60, Qt::blue);
    }

    // Boss (2倍 110×110，循环动画)
    QPixmap bossTex = m_bossPixmap[m_bossFrame];
    if (!bossTex.isNull()) {
        painter.drawPixmap(bossX, bossY, 110, 110, bossTex);
    } else {
        painter.fillRect(bossX, bossY, 110, 110, Qt::darkRed);
    }

    // 玩家子弹 (2.5倍大小 30×30)
    QPixmap bulletFrame = m_bulletPixmap[m_bulletFrame];
    for (const PlayerBullet &b : playerBullets) {
        if (b.active) {
            if (!bulletFrame.isNull())
                painter.drawPixmap((int)b.x, (int)b.y, 30, 30, bulletFrame);
            else
                painter.fillRect((int)b.x, (int)b.y, 30, 30, Qt::cyan);
        }
    }

    // Boss 子弹 (0.6倍 18×18，随机贴图)
    for (const BossBullet &b : bossBullets) {
        if (b.active) {
            QPixmap tex = m_bossBulletPixmap[b.tex];
            if (!tex.isNull())
                painter.drawPixmap((int)b.x, (int)b.y, 18, 18, tex);
            else
                painter.fillRect((int)b.x, (int)b.y, 18, 18, Qt::darkYellow);
        }
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
    painter.drawText(width() / 2 - 140, 37, QString("BOSS:").arg(bossHealth).arg(bossMaxHealth));

    // ★ 玩家血条（在 Boss 血条正下方，绿色）
    int playerBarWidth = (playerHealth * 200) / playerMaxHealth;
    painter.setPen(Qt::NoPen);
    painter.fillRect(width() / 2 - 100, 55, 200, 20, Qt::darkGray);
    painter.fillRect(width() / 2 - 100, 55, playerBarWidth, 20, Qt::green);
    painter.setPen(Qt::white);
    painter.setFont(f);
    painter.drawText(width() / 2 - 90, 70, QString("HP: ").arg(playerHealth).arg(playerMaxHealth));

    // 暂停遮罩
    if (m_paused) {
        painter.fillRect(rect(), QColor(0, 0, 0, 160));
        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setPointSize(36);
        f.setBold(true);
        painter.setFont(f);
        painter.drawText(rect(), Qt::AlignCenter, "已暂停\n按 ESC 继续");
    }
}

// ===================== 按键 =====================

void HollowKnightWidget::keyPressEvent(QKeyEvent *event)
{
    // ESC 暂停/继续
    if (event->key() == Qt::Key_Escape) {
        m_paused = !m_paused;
        if (m_paused) {
            gameTimer->stop();
        } else {
            gameTimer->start(50);
        }
        emit gamePaused(m_paused);
        update();
        return;
    }

    if (m_paused) return;

    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:        keyUp = true; break;
    case Qt::Key_S:
    case Qt::Key_Down:      keyDown = true; break;
    case Qt::Key_A:
    case Qt::Key_Left:      keyLeft = true; m_lastDirectionX = -1; break;
    case Qt::Key_D:
    case Qt::Key_Right:     keyRight = true; m_lastDirectionX = 1; break;
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
    // 玩家走路动画：移动时每0.15秒切换帧（3个50ms周期）
    if (keyLeft || keyRight) {
        m_playerFrameCounter++;
        if (m_playerFrameCounter >= 3) {
            m_playerFrameCounter = 0;
            m_playerFrame = (m_playerFrame + 1) % 4;
        }
    } else {
        m_playerFrame = 0;
        m_playerFrameCounter = 0;
    }

    m_bulletFrame = (m_bulletFrame + 1) % 2;

    m_bossFrameCounter++;
    if (m_bossFrameCounter >= 6) {
        m_bossFrameCounter = 0;
        m_bossFrame = (m_bossFrame + 1) % 4;
    }

    movePlayer();
    updateBoss();
    update();
}
