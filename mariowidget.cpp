#include "mariowidget.h"
#include <QPainter>
#include <QTime>
#include <cmath>

// ★ 实时读取键盘状态的辅助函数
static bool isKeyDown(Qt::Key qtKey)
{
    int vk = 0;
    switch (qtKey) {
    case Qt::Key_A:       vk = 'A';       break;
    case Qt::Key_D:       vk = 'D';       break;
    case Qt::Key_Left:    vk = VK_LEFT;   break;
    case Qt::Key_Right:   vk = VK_RIGHT;  break;
    case Qt::Key_W:       vk = 'W';       break;
    case Qt::Key_Space:   vk = VK_SPACE;  break;
    case Qt::Key_J:       vk = 'J';       break;
    default: return false;
    }
    return GetAsyncKeyState(vk) & 0x8000;
}

MarioWidget::MarioWidget(QWidget *parent, int difficulty, QSize size)
    : QWidget(parent)
    , difficulty(difficulty)
    , playerWorldX(0)
    , playerY(300)
    , playerVelocityX(0)
    , playerVelocityY(0)
    , isOnGround(false)
    , jumpCount(0)
    , invincibleCount(2)    // 金身护体2次
    , keyAttack(false)
    , lastDirection(1)
    , facingDirection(1)
    , score(0)
    , coins(0)
    , enemiesKilled(0)
    , cameraX(0)
    , m_gameOver(false)
{
    setFixedSize(size);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    initLevel();

    gameTimer = new QTimer(this);
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateGame()));
    gameTimer->start(30);
}

MarioWidget::~MarioWidget() {}

void MarioWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setFocus();
}

void MarioWidget::setDifficulty(int difficulty)
{
    this->difficulty = difficulty;
}

void MarioWidget::reset()
{
    playerWorldX = 0;
    playerY = 300;
    playerVelocityX = 0;
    playerVelocityY = 0;
    isOnGround = false;
    jumpCount = 0;
    invincibleCount = 2;    // ★ 重置护体次数
    keyAttack = false;
    lastDirection = 1;
    facingDirection = 1;
    score = 0;
    coins = 0;
    enemiesKilled = 0;
    cameraX = 0;
    m_gameOver = false;
    initLevel();
}

void MarioWidget::initLevel()
{
    coinList.clear();
    enemies.clear();
    platforms.clear();

    // 地面
    platforms.append({0,    GROUND_Y, 500, 50});
    platforms.append({580,  GROUND_Y, 500, 50});
    platforms.append({1160, GROUND_Y, 500, 50});
    platforms.append({1740, GROUND_Y, 500, 50});
    platforms.append({2320, GROUND_Y, 500, 50});
    platforms.append({2900, GROUND_Y, 400, 50});
    platforms.append({3380, GROUND_Y, 400, 50});
    platforms.append({3800, GROUND_Y, 250, 50});

    // 高空平台
    qsrand((uint)QTime::currentTime().msec());
    for (int i = 0; i < 20; i++) {
        double px = 200 + qrand() % (WORLD_WIDTH - 400);
        double py = 250 + qrand() % 200;
        bool overlap = false;
        for (const Platform &p : platforms) {
            if (std::abs(px - p.x) < 200 && std::abs(py - p.y) < 100) {
                overlap = true; break;
            }
        }
        if (!overlap)
            platforms.append({px, py, (double)(60 + qrand() % 100), 20.0});
    }

    // 金币
    int coinCount = 10 + difficulty * 3;
    for (int i = 0; i < coinCount; i++)
        coinList.append({(double)(200 + qrand() % (WORLD_WIDTH - 400)),
                         (double)(200 + qrand() % 300), false});

    // 敌人（血量2）
    int enemyCount = 2 + difficulty * 2;
    for (int i = 0; i < enemyCount; i++) {
        double ex = 300.0 + (double)i * (WORLD_WIDTH / (enemyCount + 1));
        enemies.append({ex, (double)(GROUND_Y - 40),
                        (double)((qrand() % 2 == 0) ? 2 : -2), 2, true});
    }
}

bool MarioWidget::checkCollision(const QRect &rect1, const QRect &rect2)
{
    return rect1.intersects(rect2);
}

void MarioWidget::movePlayer()
{
    const double GRAVITY = 0.8;
    const double MAX_FALL_SPEED = 15;
    double MOVE_SPEED = 3.0;
    if (difficulty == 2) MOVE_SPEED = 4.0;
    else if (difficulty == 3) MOVE_SPEED = 5.0;

    // ===== 用 GetAsyncKeyState 直接读键盘 =====
    bool keyLeft  = isKeyDown(Qt::Key_A) || isKeyDown(Qt::Key_Left);
    bool keyRight = isKeyDown(Qt::Key_D) || isKeyDown(Qt::Key_Right);

    if (keyLeft && keyRight) {
        playerVelocityX = lastDirection * MOVE_SPEED;
        facingDirection = lastDirection;
    } else if (keyLeft) {
        playerVelocityX = -MOVE_SPEED;
        facingDirection = -1;
        lastDirection = -1;
    } else if (keyRight) {
        playerVelocityX = MOVE_SPEED;
        facingDirection = 1;
        lastDirection = 1;
    } else {
        playerVelocityX = 0;
    }

    // ===== 重力 =====
    double prevBottom = playerY + 40;
    playerVelocityY += GRAVITY;
    if (playerVelocityY > MAX_FALL_SPEED) playerVelocityY = MAX_FALL_SPEED;

    playerWorldX += playerVelocityX;
    playerY += playerVelocityY;
    if (playerWorldX < 0) playerWorldX = 0;
    if (playerWorldX > WORLD_WIDTH) playerWorldX = WORLD_WIDTH;

    // ===== 平台碰撞 =====
    isOnGround = false;
    for (const Platform &p : platforms) {
        double centerX = playerWorldX + 15;
        if (centerX < p.x || centerX > p.x + p.width)
            continue;
        if (playerVelocityY >= 0 &&
            prevBottom <= p.y + 15 &&
            playerY + 40 >= p.y - 5 &&
            playerY + 40 <= p.y + p.height + 10) {
            playerY = p.y - 40;
            playerVelocityY = 0;
            isOnGround = true;
            jumpCount = 0;
        }
    }

    if (playerY > height() + 100) {
        if (!m_gameOver) {
            m_gameOver = true;
            gameTimer->stop();
            emit gameLost();
        }
        return;
    }

    // ===== 摄像机 =====
    cameraX = playerWorldX - PLAYER_SCREEN_X;
    if (cameraX < 0) cameraX = 0;
    if (cameraX > WORLD_WIDTH - width()) cameraX = WORLD_WIDTH - width();

    QRect playerRect(playerWorldX, playerY, 30, 40);

    // ===== 金币 =====
    for (Coin &c : coinList) {
        if (!c.collected) {
            QRect coinRect(c.x, c.y, 20, 20);
            if (checkCollision(playerRect, coinRect)) {
                c.collected = true;
                coins++;
                score++;
            }
        }
    }

    // ===== 攻击状态（用 GetAsyncKeyState 读 J 键）=====
    bool keyAttackHeld = isKeyDown(Qt::Key_J);
    if (keyAttack) keyAttackHeld = true;  // 允许 Qt 事件触发一次后保持

    // ===== 敌人 =====
    for (Enemy &e : enemies) {
        if (!e.alive) continue;

        e.x += e.velocityX;
        if (e.x < 0 || e.x > WORLD_WIDTH - 30)
            e.velocityX *= -1;

        QRect enemyRect(e.x, e.y, 30, 30);

        // 攻击检测
        if (keyAttackHeld) {
            double dx = (e.x + 15) - (playerWorldX + 15);
            double dy = (e.y + 15) - (playerY + 20);

            bool inRange = false;
            if (facingDirection == 1 && dx > 0 && dx < 70) inRange = true;
            if (facingDirection == -1 && dx < 0 && dx > -70) inRange = true;

            if (inRange && std::abs(dy) < 60) {
                e.health--;
                if (facingDirection == 1) e.x += 30;
                else e.x -= 30;

                if (e.health <= 0) {
                    e.alive = false;
                    enemiesKilled++;
                    score += 2;
                }
                continue;
            }
        }

        // 碰撞检测
        if (checkCollision(playerRect, enemyRect)) {
            if (playerVelocityY > 0 && playerY + 40 < e.y + 15) {
                e.health--;
                playerVelocityY = -10;
                if (e.health <= 0) {
                    e.alive = false;
                    enemiesKilled++;
                    score += 2;
                }
            } else {
                // ★ 金身护体：前 invincibleCount 次不杀死玩家
                if (invincibleCount > 0) {
                    invincibleCount--;
                    // 把敌人推开，不再重叠
                    if (facingDirection == 1) e.x -= 60;
                    else e.x += 60;
                } else {
                    if (!m_gameOver) {
                        m_gameOver = true;
                        gameTimer->stop();
                        emit gameLost();
                    }
                    return;
                }
            }
        }
    }

    // ===== 到达终点 =====
    // ★ 胜利分数按难度：简单15、中等20、困难30
    int targetScore = (difficulty == 1) ? 15 : (difficulty == 2) ? 20 : 30;
    if (score >= targetScore && !m_gameOver) {
        m_gameOver = true;
        gameTimer->stop();
        emit gameWon();
    }
}

void MarioWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), QColor(135, 206, 235));

    for (const Platform &p : platforms) {
        double sx = p.x - cameraX;
        if (sx + p.width < -50 || sx > width() + 50) continue;
        painter.fillRect(sx, p.y, p.width, p.height, Qt::green);
    }

    for (const Coin &c : coinList) {
        double sx = c.x - cameraX;
        if (sx < -50 || sx > width() + 50) continue;
        if (!c.collected) {
            painter.setBrush(Qt::yellow);
            painter.drawEllipse(sx, c.y, 20, 20);
        }
    }

    for (const Enemy &e : enemies) {
        double sx = e.x - cameraX;
        if (sx < -50 || sx > width() + 50) continue;
        if (e.alive) painter.fillRect(sx, e.y, 30, 30, Qt::red);
    }

    painter.fillRect(PLAYER_SCREEN_X, playerY, 30, 40, Qt::blue);

    // 攻击特效（只要按住 J 就显示）
    bool keyAttackHeld = isKeyDown(Qt::Key_J) || keyAttack;
    if (keyAttackHeld) {
        int ax = (facingDirection == 1) ? PLAYER_SCREEN_X + 25 : PLAYER_SCREEN_X - 55;
        painter.fillRect(ax, playerY - 20, 50, 80, QColor(255, 255, 0, 80));
    }

    painter.setPen(Qt::black);
    QFont font;
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);
    int targetScore = (difficulty == 1) ? 15 : (difficulty == 2) ? 20 : 30;
    painter.drawText(10, 25, QString::fromUtf8("\xe5\xbe\x97\xe5\x88\x86: %1 / %2").arg(score).arg(targetScore));
    painter.drawText(10, 65, QString::fromUtf8("\xe6\x95\x8c\xe4\xba\xba: %1").arg(enemiesKilled));
    if (invincibleCount > 0)    // ★ 显示剩余护体次数
    painter.drawText(10, 85, QString::fromUtf8("\xe2\x99\xaa \xe9\x87\x91\xe8\xba\xab: %1 \xe6\xac\xa1").arg(invincibleCount));
    painter.drawText(10, 45, QString::fromUtf8("\xe9\x87\x91\xe5\xb8\x81: %1").arg(coins));
    painter.drawText(10, 65, QString::fromUtf8("\xe6\x95\x8c\xe4\xba\xba: %1").arg(enemiesKilled));
}

// ===================== ★ 按键处理（简化，只处理跳和攻击）=====================
// 左/右键不再通过 Qt 事件处理，改用 GetAsyncKeyState 直接读键盘

void MarioWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_Up:
    case Qt::Key_Space:
        if (jumpCount < 2) {
            playerVelocityY = -16;
            jumpCount++;
        }
        break;
    case Qt::Key_J:
        keyAttack = true;
        break;
    }
}

void MarioWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_J:
        keyAttack = false;
        break;
    }
}

void MarioWidget::updateGame()
{
    movePlayer();
    update();
}
