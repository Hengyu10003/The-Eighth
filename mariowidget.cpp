#include "mariowidget.h"
#include <QPainter>
#include <QTime>
#include <cmath>
#include <QPushButton>

//构造函数
MarioWidget::MarioWidget(QWidget *parent, int difficulty, QSize size)
    : QWidget(parent)
    , difficulty(difficulty)
    , playerWorldX(200)
    , playerY(GROUND_Y - 51)
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
    , m_keyLeft(false)
    , m_keyRight(false)
    , m_enemyPixmap(GUAI)
    , m_platformPixmap(ZHUAN)
    , m_obstaclePixmap(ZHUAN2)
    , m_coinPixmap1(BI1)
    , m_coinPixmap2(BI2)
    , m_bgPixmap(MARIO_B)
    , m_playerWidth(51)
    , m_playerHeight(51)
    , m_coinFrame(0)
    , m_coinFrameCounter(0)
    , m_playerFrame(0)
    , m_playerFrameCounter(0)
    , m_paused(false)
{
//固定界面大小
    setFixedSize(size);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
//玩家=========================================================================================================
//玩家尺寸
    m_playerWidth = 51;
    m_playerHeight = 51;

// 加载玩家动画图片
    m_playerPixmapL[0].load(TUAN1_l);
    m_playerPixmapL[1].load(TUAN2_l);
    m_playerPixmapL[2].load(TUAN3_l);
    m_playerPixmapL[3].load(TUAN4_l);
    m_playerPixmapR[0].load(TUAN1_r);
    m_playerPixmapR[1].load(TUAN2_r);
    m_playerPixmapR[2].load(TUAN3_r);
    m_playerPixmapR[3].load(TUAN4_r);

    initLevel();
//更新函数============================================================================================================
    gameTimer = new QTimer(this);
    connect(gameTimer, SIGNAL(timeout()), this, SLOT(updateGame()));
    gameTimer->start(30);

// 返回主菜单按钮=========================================================================================================
    QPushButton *backBtn = new QPushButton(this);
    backBtn->setGeometry(1090, 0, 98, 59);
    backBtn->setText("返回");
    backBtn->setStyleSheet("QPushButton { border-image: url(" ZHUAN "); color: black; font-weight: bold; }"
                           "QPushButton:hover { background: rgba(255,255,255,100); }");
    backBtn->setFocusPolicy(Qt::NoFocus);
    connect(backBtn, SIGNAL(clicked()), this, SIGNAL(returnToMenu()));
}
//析构函数
MarioWidget::~MarioWidget() {}

//抢占键盘焦点Qt直接使用
void MarioWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setFocus();
}

// 初始化
void MarioWidget::reset()
{
    playerWorldX = 200;                                      // 玩家回到起始x位置
    playerY = GROUND_Y - m_playerHeight;                     // 玩家站到地面上
    playerVelocityX = 0;                                     // 水平速度归零
    playerVelocityY = 0;                                     // 垂直速度归零（不跳不落）
    isOnGround = false;                                      // 不在地上（刚重置时腾空落下）
    jumpCount = 0;                                           // 跳跃次数归零
    invincibleCount = 2;                                     // 金身护体次数重置为2次
    keyAttack = false;                                       // 攻击键松开
    lastDirection = 1;                                       // 上次移动方向朝右
    facingDirection = 1;                                     // 当前朝向朝右
    score = 0;                                               // 分数归零
    coins = 0;                                               // 金币归零
    enemiesKilled = 0;                                       // 杀敌数归零
    cameraX = 0;                                             // 摄像机拉到最左边
    m_gameOver = false;                                      // 游戏未结束
    m_keyLeft = false;                                       // 左移键松开
    m_keyRight = false;                                      // 右移键松开
    initLevel();                                             // 重新生成关卡地形、金币、敌人
}

// 生成关卡里所有的东西：地面、障碍物、高台、金币、敌人
void MarioWidget::initLevel()
{
//清除
    coinList.clear();                                        // 清掉上一局的金币
    enemies.clear();                                         // 清掉上一局的敌人
    platforms.clear();                                       // 清掉上一局的地面
    obstacles.clear();                                       // 清掉上一局的障碍物
//初始化
    qsrand((uint)QTime::currentTime().msec());               // 用当前时间做随机种子，保证每局不一样

// ===== 地面：8段地面，段与段中间有空隙 =====
    double baseGap = 80.0;                                   // 地面之间的基本空隙宽度
    double groundWidths[] = {500, 500, 500, 500, 500, 400, 400, 250};  // 8段地面各自的宽度
    int groundCount = sizeof(groundWidths) / sizeof(groundWidths[0]);
    double currentX = 0;                                     // 当前这段地面从哪开始
    for (int i = 0; i < groundCount; i++) {
        platforms.append({currentX, (double)GROUND_Y, groundWidths[i], 50.0});  // 塞一段地面

        // 难度越高，地面之间的空隙越大
        double gapMult = 1.0;
        if (difficulty == 2) {
            gapMult = 1.0 + (qrand() % 301) / 1000.0;       // 中等难度：空隙放大1.0~1.3倍
        } else if (difficulty == 3) {
            gapMult = 1.2 + (qrand() % 301) / 1000.0;       // 困难难度：空隙放大1.2~1.5倍
        }
        currentX += groundWidths[i] + baseGap * gapMult;     // 跳到下一段地面的起点
    }

//  再放障碍物：贴在地面上的方块，玩家得跳过去 =====
    int obstacleCount = (difficulty == 1) ? 2 : (difficulty == 2) ? 5 : 7;  // 简单放2个，中等5个，困难7个
    int minSize = m_playerWidth;                             // 最小跟玩家一样大（51像素）
    int maxSize = (int)(m_playerWidth * 1.5);                // 最大是玩家的1.5倍（76像素）
    for (int i = 0; i < obstacleCount; i++) {
        int obsSize = minSize + qrand() % (maxSize - minSize + 1);  // 随机取51~76之间的尺寸
        // 试着放30次，找个不跟别的障碍物重叠的位置
        bool placed = false;
        for (int attempt = 0; attempt < 30 && !placed; attempt++) {
            int pi = qrand() % groundCount;                  // 随机挑一段地面
            const Platform &p = platforms[pi];
            double ox = p.x + qrand() % (int)(p.width - obsSize);  // 在这段地面的范围内随机横坐标
            double oy = p.y - obsSize;                       // 障碍物的底边贴着地面
            QRect newRect(ox, oy, obsSize, obsSize);
            bool overlaps = false;
            for (const Obstacle &o : obstacles) {            // 检查跟已经放好的有没有重叠
                QRect existRect(o.x, o.y, o.size, o.size);
                if (newRect.intersects(existRect.adjusted(-20, -20, 20, 20))) {  // 四周各留20像素余量
                    overlaps = true; break;
                }
            }
            if (!overlaps) {
                obstacles.append({ox, oy, (double)obsSize});
                placed = true;
            }
        }
        if (!placed) {                                       // 试了30次还放不下，随便找个位置硬塞进去
            int pi = qrand() % groundCount;
            const Platform &p = platforms[pi];
            double ox = p.x + qrand() % (int)(p.width - obsSize);
            double oy = p.y - obsSize;
            obstacles.append({ox, oy, (double)obsSize});
        }
    }

// ===== 加一些高空平台，玩家可以跳上去 =====
    for (int i = 0; i < 20; i++) {
        double px = 200 + qrand() % (WORLD_WIDTH - 400);     // 横坐标：200~3800之间随机
        double py = 250 + qrand() % 200;                     // 纵坐标：250~450之间（离地面有一段距离）
        bool overlap = false;
        for (const Platform &p : platforms) {
            if (std::abs(px - p.x) < 200 && std::abs(py - p.y) < 100) {  // 离已有平台太近就跳过
                overlap = true; break;
            }
        }
        if (!overlap)
            platforms.append({px, py, (double)(60 + qrand() % 100), 20.0});  // 宽度60~160，高度固定20
    }

// ===== 撒金币：分数的主要来源 =====
    int coinCount = 10 + difficulty * 3;                     // 简单13个，中等16个，困难19个
    for (int i = 0; i < coinCount; i++)
        coinList.append({(double)(200 + qrand() % (WORLD_WIDTH - 400)),  // x坐标200~3800
                         (double)(200 + qrand() % 300), false});         // y坐标200~500，还没被捡

// ===== 放敌人：来回巡逻，血量2，得打两下才死 =====
    int enemyCount = 4 + difficulty * 2;                     // 简单6个，中等8个，困难10个
    for (int i = 0; i < enemyCount; i++) {
        double ex = 300.0 + (double)i * (WORLD_WIDTH / (enemyCount + 1));  // 均匀分布在整条路上
        enemies.append({ex, (double)(GROUND_Y - 40),                      // 站在地面上
                        (double)((qrand() % 2 == 0) ? 2 : -2),            // 随机朝右走(+2)或朝左走(-2)
                        2, true});                                         // 血量2，活着
    }
}

// 检测两个矩形是否撞上了（重叠就返回true）
bool MarioWidget::checkCollision(const QRect &rect1, const QRect &rect2)
{
    return rect1.intersects(rect2);                          // Qt自带的矩形相交判断
}

// 每一帧更新玩家位置、处理各种碰撞
void MarioWidget::movePlayer()
{
//速度控制
    const double GRAVITY = 0.8; // 每帧往下拉的重力加速度
    const double MAX_FALL_SPEED = 15;// 下落速度封顶，防止穿模
    double MOVE_SPEED = (difficulty == 1) ? 5.0 : (difficulty == 2) ? 6.5 : 8.0;  // 难度越高跑得越快

    // ===== ① 左右移动：按哪个键往哪走，两个都按就保持上次方向 =====
    if (m_keyLeft && m_keyRight) {                           // 左右同时按，保持原方向
        playerVelocityX = lastDirection * MOVE_SPEED;
        facingDirection = lastDirection;
    } else if (m_keyLeft) {                                  // 按左
        playerVelocityX = -MOVE_SPEED;//向左移动
        facingDirection = -1;//向左攻击
        lastDirection = -1;//最后移动方向
    } else if (m_keyRight) {                                 // 按右
        playerVelocityX = MOVE_SPEED;
        facingDirection = 1;
        lastDirection = 1;
    } else {
        playerVelocityX = 0;                                 // 啥也没按，停住
    }

    // ===== ② 重力：每帧往下拽，但不能无限制加速 =====
    double prevBottom = playerY + m_playerHeight;            // 移动前的脚底位置（用于碰撞判断）
    playerVelocityY += GRAVITY;                              // 重力加速下落
    if (playerVelocityY > MAX_FALL_SPEED) playerVelocityY = MAX_FALL_SPEED;  // 限制最大下落速度

    playerWorldX += playerVelocityX;//（地图位置） 水平移动
    playerY += playerVelocityY;                              // 垂直移动
    if (playerWorldX < 0) playerWorldX = 0;                  // 别跑出左边界
    if (playerWorldX > WORLD_WIDTH) playerWorldX = WORLD_WIDTH;  // 别跑出右边界

    // ===== ③ 站在平台上：玩家脚底跟平台顶面接触时，把玩家托住 =====
    isOnGround = false;
    for (const Platform &p : platforms) {
        // 玩家跟平台在水平方向上没重叠，跳过
        if (playerWorldX + m_playerWidth <= p.x || playerWorldX >= p.x + p.width)
            continue;
        // 玩家在往下掉，上一帧脚底在平台顶附近，这一帧脚底穿过平台顶 → 站在平台上
        if (playerVelocityY >= 0 &&
            prevBottom <= p.y + 15 &&
            playerY + m_playerHeight >= p.y - 5 &&
            playerY + m_playerHeight <= p.y + p.height + 10) {
            playerY = p.y - m_playerHeight;                  // 把玩家拉到平台上面
            playerVelocityY = 0;                             // 竖直速度归零
            isOnGround = true;                               // 标记"脚踩实地"
            jumpCount = 0;                                   // 重置跳跃次数，可以再跳了
        }
    }

    // 掉到深渊里了 → 游戏结束
    if (playerY > height() + 100) {
        if (!m_gameOver) {
            m_gameOver = true;
            gameTimer->stop();
            emit gameLost();
        }
        return;
    }

    // ===== ④ 跟障碍物碰撞：站上去或者被挡住 =====
    QRect playerRect(playerWorldX, playerY, m_playerWidth, m_playerHeight);
    for (const Obstacle &o : obstacles) {
        QRect obsRect(o.x, o.y, o.size, o.size);

        // ④-a 站在障碍物上面（跟站平台同理）
        if (playerVelocityY >= 0 &&                   // 正在下落/静止（不往上跳才可能是踩到）
            playerRect.right() > obsRect.left() + 3 && // 玩家右边在障碍物左边右边3像素以上（水平有重叠）
            playerRect.left() < obsRect.right() - 3) { // 玩家左边在障碍物右边左边3像素以上（水平确实重叠）
            double prevBottom = playerY - playerVelocityY + m_playerHeight; // 上一帧的脚底位置
            if (prevBottom <= o.y + 10 &&               // 上一帧脚在障碍物顶部附近（从上面落下来的）
                playerY + m_playerHeight >= o.y - 3 &&   // 这一帧脚到了障碍物顶部（碰到了）
                playerY + m_playerHeight <= o.y + o.size * 0.5) { // 脚没超过障碍物一半高度（防止侧面误判）
                playerY = o.y - m_playerHeight;          // 把玩家放回障碍物顶部
                playerVelocityY = 0;                     // 下落速度归零
                isOnGround = true;                       // 标记为站在地上
                jumpCount = 0;                           // 重置跳跃次数（可以重新跳）
                playerRect = QRect(playerWorldX, playerY, m_playerWidth, m_playerHeight); // 更新碰撞盒
                continue;                                // 跳过后续碰撞检测
            }
        }

        // ④-b 水平方向被障碍物挡住（撞墙了）
        if (playerRect.right() > obsRect.left() && playerRect.left() < obsRect.right() &&//矩形重叠条件
            playerRect.bottom() > obsRect.top() + 5 && playerRect.top() + m_playerHeight * 0.4 < obsRect.bottom()) {
            if (playerVelocityX > 0) {
                playerWorldX = obsRect.left() - m_playerWidth;  // 把玩家推到障碍物左边
            } else if (playerVelocityX < 0) {
                playerWorldX = obsRect.right();                 // 把玩家推到障碍物右边
            }
            playerVelocityX = 0;                             // 水平速度归零
            playerRect = QRect(playerWorldX, playerY, m_playerWidth, m_playerHeight);
        }
    }

    // ===== ⑤ 摄像机：玩家始终保持在屏幕左侧200像素的位置，场景跟着滚动 =====
    cameraX = playerWorldX - PLAYER_SCREEN_X;
    if (cameraX < 0) cameraX = 0;                            // 别滚出最左边
    if (cameraX > WORLD_WIDTH - width()) cameraX = WORLD_WIDTH - width();  // 别滚出最右边

    // ===== ⑥ 捡金币：碰到就算捡到，分数+1 =====
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

    // ===== ⑦ 打敌人：按J键攻击 or 踩踏 =====
    bool keyAttackHeld = keyAttack;

    for (Enemy &e : enemies) {
        if (!e.alive) continue;                              // 死了的不管

        e.x += e.velocityX;                                  // 敌人巡逻移动
        if (e.x < 0 || e.x > WORLD_WIDTH - 30)               // 碰到边界掉头
            e.velocityX *= -1;

        QRect enemyRect(e.x, e.y, 30, 30);

        // ⑦-a 按J攻击：面朝方向70像素范围内、高度差小于60的敌人扣血
        if (keyAttackHeld) {
            double dx = (e.x + 15) - (playerWorldX + m_playerWidth / 2.0);
            double dy = (e.y + 15) - (playerY + m_playerHeight / 2.0);

            bool inRange = false;
            if (facingDirection == 1 && dx > 0 && dx < 70) inRange = true;   // 朝右打右边的敌人
            if (facingDirection == -1 && dx < 0 && dx > -70) inRange = true; // 朝左打左边的敌人

            if (inRange && std::abs(dy) < 60) {
                e.health--;                                  // 敌人掉血
                if (facingDirection == 1) e.x += 30;         // 击退效果
                else e.x -= 30;

                if (e.health <= 0) {                         // 打死了
                    e.alive = false;
                    enemiesKilled++;
                    score += 2;
                }
                continue;
            }
        }

        // ⑦-b 碰到敌人：从上面踩→敌人扣血；从侧面撞→玩家扣"金身"次数
        if (checkCollision(playerRect, enemyRect)) {
            if (playerVelocityY > 0 && playerY + m_playerHeight < e.y + 15) {  // 从上面踩
                e.health--;
                playerVelocityY = -10;                       // 踩完之后弹起来
                if (e.health <= 0) {
                    e.alive = false;
                    enemiesKilled++;
                    score += 2;
                }
            } else {
                // 金身护体：前 invincibleCount 次不杀死玩家
                if (invincibleCount > 0) {
                    invincibleCount--;                       // 消耗一次护体
                    if (facingDirection == 1) e.x -= 60;     // 把敌人推开
                    else e.x += 60;
                } else {                                     // 护体用完，真的死了
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

    // ===== ⑧ 到达胜利分数 =====
    int targetScore = (difficulty == 1) ? 15 : (difficulty == 2) ? 20 : 30;  // 简单15分，中等20分，困难30分
    if (score >= targetScore && !m_gameOver) {
        m_gameOver = true;
        gameTimer->stop();
        emit gameWon();
    }
}

//绘图============================================================================================================
void MarioWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
//背景
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(rect(), m_bgPixmap);
    } else {
        painter.fillRect(rect(), QColor(135, 206, 235));
    }
//地面
    for (const Platform &p : platforms) {
        double sx = p.x - cameraX;
        if (sx + p.width < -50 || sx > width() + 50) continue;
        if (!m_platformPixmap.isNull()) {
            painter.drawPixmap(sx, p.y, p.width, p.height, m_platformPixmap);
        } else {
            painter.fillRect(sx, p.y, p.width, p.height, Qt::green);
        }
    }
//金币
    for (const Coin &c : coinList) {
        double sx = c.x - cameraX;
        if (sx < -50 || sx > width() + 50) continue;
        if (!c.collected) {
            QPixmap &coinPix = (m_coinFrame == 0) ? m_coinPixmap1 : m_coinPixmap2;
            if (!coinPix.isNull()) {
                painter.drawPixmap(sx, c.y, 20, 20, coinPix);
            } else {
                painter.setBrush(Qt::yellow);
                painter.drawEllipse(sx, c.y, 20, 20);
            }
        }
    }
//敌人
    for (const Enemy &e : enemies) {
        double sx = e.x - cameraX;
        if (sx < -50 || sx > width() + 50) continue;
        if (!e.alive) continue;
        if (!m_enemyPixmap.isNull()) {
            painter.drawPixmap(sx, e.y, 30, 30, m_enemyPixmap);
        } else {
            painter.fillRect(sx, e.y, 30, 30, Qt::red);
        }
    }

// 障碍物
    for (const Obstacle &o : obstacles) {
        double sx = o.x - cameraX;
        if (sx + o.size < -50 || sx > width() + 50) continue;
        if (!m_obstaclePixmap.isNull()) {
            painter.drawPixmap(sx, o.y, o.size, o.size, m_obstaclePixmap);
        } else {
            painter.fillRect(sx, o.y, o.size, o.size, QColor(139, 69, 19)); // 棕色回退
        }
    }

// 玩家动画：根据朝向选择对应帧
    QPixmap currentFrame;
    if (facingDirection == 1) {
        currentFrame = m_playerPixmapR[m_playerFrame];
    } else {
        currentFrame = m_playerPixmapL[m_playerFrame];
    }
    if (!currentFrame.isNull()) {
        painter.drawPixmap(PLAYER_SCREEN_X, playerY, m_playerWidth, m_playerHeight, currentFrame);
    } else {
        painter.fillRect(PLAYER_SCREEN_X, playerY, m_playerWidth, m_playerHeight, Qt::blue);
    }

// 攻击特效（只要按住 J 就显示）
    if (keyAttack) {
        int ax = (facingDirection == 1) ? PLAYER_SCREEN_X + m_playerWidth / 2 : PLAYER_SCREEN_X - 60 + m_playerWidth / 2;
        painter.setBrush(QColor(255, 255, 0, 80));//颜色
        painter.setPen(Qt::NoPen);//不要边框
        painter.drawEllipse(ax, playerY + m_playerHeight / 2 - 30, 60, 60);//（水平，垂直，大小）
    }

//HUD
    painter.setPen(Qt::black);
    QFont font;
    font.setPointSize(12);//字号
    font.setBold(true);//加粗
    painter.setFont(font);//字体
    int targetScore = (difficulty == 1) ? 15 : (difficulty == 2) ? 20 : 30;
    painter.drawText(10, 25, QString("得分: %1 / %2").arg(score).arg(targetScore));  // 左上角显示分数
    painter.drawText(10, 65, QString("金币: %1").arg(coins));
    painter.drawText(10, 105, QString("敌人: %1").arg(enemiesKilled));
    if (invincibleCount > 0)
        painter.drawText(10, 145, QString("金身: %1 次").arg(invincibleCount));   // 还有几次免死金牌

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

// ===================== 按键处理 =====================

void MarioWidget::keyPressEvent(QKeyEvent *event)
{
// ESC 暂停/继续------------------------------------------------------------------
    if (event->key() == Qt::Key_Escape) {
        m_paused = !m_paused;
        if (m_paused) {
            gameTimer->stop();
        } else {
            gameTimer->start(30);
        }
        emit gamePaused(m_paused);
        update();
        return;
    }
//移动事件----------------------------------------------------------------------
    if (m_paused) return;

    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        m_keyLeft = true;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_keyRight = true;
        break;
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
    case Qt::Key_A:
    case Qt::Key_Left:
        m_keyLeft = false;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_keyRight = false;
        break;
    case Qt::Key_J:
        keyAttack = false;
        break;
    }
}

void MarioWidget::updateGame()
{
// 金币动画：每0.2秒切换帧（约7个30ms周期）
    m_coinFrameCounter++;
    if (m_coinFrameCounter >= 7) {
        m_coinFrameCounter = 0;
        m_coinFrame = (m_coinFrame + 1) % 2;
    }

// 玩家走路动画：移动时每0.15秒切换帧（约5个30ms周期）
    if (playerVelocityX != 0) {
        m_playerFrameCounter++;
        if (m_playerFrameCounter >= 5) {
            m_playerFrameCounter = 0;
            m_playerFrame = (m_playerFrame + 1) % 4;
        }
    } else {
        m_playerFrame = 0;
        m_playerFrameCounter = 0;
    }

    movePlayer();
    update();
}
