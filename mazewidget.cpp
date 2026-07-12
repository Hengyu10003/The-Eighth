#include "mazewidget.h"
#include <QPainter>
#include <QTime>
#include <QStack>
#include <QPair>
#include <QDebug>
#include <QKeyEvent>
#include <QPushButton>
#include <QQueue>

MazeWidget::MazeWidget(QWidget *parent, int level, QSize size)
    : QWidget(parent)
    , level(level)
    , playerX(1)
    , playerY(1)
    , maze(nullptr)
{
    setFixedSize(size);
    setFocusPolicy(Qt::StrongFocus);  // 必须有，否则键盘不响应

    switch (level) {
    case 1:
        mazeWidth = 15;
        mazeHeight = 11;
        break;
    case 2:
        mazeWidth = 21;
        mazeHeight = 15;
        break;
    case 3:
        mazeWidth = 27;
        mazeHeight = 19;
        break;
    default:
        mazeWidth = 15;
        mazeHeight = 11;
        break;
    }

    cellSize = qMin(width() / mazeWidth, height() / mazeHeight);
    generateMaze();

    // 加载玩家动画（向右）
    m_playerPixmapR[0].load(TUAN1_r);
    m_playerPixmapR[1].load(TUAN2_r);
    m_playerPixmapR[2].load(TUAN3_r);
    m_playerPixmapR[3].load(TUAN4_r);
    m_playerPixmapL[0].load(TUAN1_l);
    m_playerPixmapL[1].load(TUAN2_l);
    m_playerPixmapL[2].load(TUAN3_l);
    m_playerPixmapL[3].load(TUAN4_l);
    m_playerFrame = 0;
    m_lastDirectionX = 1;
    m_doorPixmap.load(DOOR);
    m_wallPixmap.load(WALL);
    m_bgPixmap.load(MIGONG);

    animationTimer = new QTimer(this);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(animatePlayer()));
    animationTimer->start(150);

    // 关键修复：打开时自动获取焦点，保证按键立刻能用
    setFocus();

    // 返回主菜单按钮
    QPushButton *backBtn = new QPushButton(this);
    backBtn->setGeometry(1050, 0, 100, 60);
    backBtn->setText("返回");
    backBtn->setStyleSheet("QPushButton { border-image: url(" WALL "); color: black; font-weight: bold; }"
                           "QPushButton:hover { background: rgba(255,255,255,100); }");
    backBtn->setFocusPolicy(Qt::NoFocus);
    connect(backBtn, SIGNAL(clicked()), this, SIGNAL(returnToMenu()));

    // 自动寻路按钮
    QPushButton *autoBtn = new QPushButton(this);
    autoBtn->setGeometry(1050, 65, 100, 60);
    autoBtn->setText("自动");
    autoBtn->setStyleSheet("QPushButton { border-image: url(" WALL "); color: black; font-weight: bold; }"
                           "QPushButton:hover { background: rgba(255,255,255,100); }");
    autoBtn->setFocusPolicy(Qt::NoFocus);
    connect(autoBtn, SIGNAL(clicked()), this, SLOT(onAutoPath()));

    // 自动寻路定时器
    m_autoPathTimer = new QTimer(this);
    connect(m_autoPathTimer, SIGNAL(timeout()), this, SLOT(autoStep()));
    m_autoPathIndex = 0;
    m_paused = false;
    m_autoPathWasRunning = false;
}

MazeWidget::~MazeWidget()
{
    if (maze) {
        for (int i = 0; i < mazeHeight; i++)
            delete[] maze[i];
        delete[] maze;
    }
}

void MazeWidget::setLevel(int level)
{
    this->level = level;
    switch (level) {
    case 1:
        mazeWidth = 15; mazeHeight = 11; break;
    case 2:
        mazeWidth = 21; mazeHeight = 15; break;
    case 3:
        mazeWidth = 27; mazeHeight = 19; break;
    default:
        mazeWidth = 15; mazeHeight = 11; break;
    }
    cellSize = qMin(width() / mazeWidth, height() / mazeHeight);
}

void MazeWidget::reset()
{
    playerX = 1;
    playerY = 1;
    generateMaze();
    update();
}

void MazeWidget::generateMaze()
{
    if (maze) {
        for (int i = 0; i < mazeHeight; i++)
            delete[] maze[i];
        delete[] maze;
    }

    maze = new bool*[mazeHeight];
    for (int i = 0; i < mazeHeight; i++) {
        maze[i] = new bool[mazeWidth];
        for (int j = 0; j < mazeWidth; j++)
            maze[i][j] = true;
    }

    qsrand(QTime::currentTime().msec());

    QStack<QPair<int, int>> stack;
    maze[1][1] = false;
    stack.push({1, 1});

    while (!stack.isEmpty()) {
        auto current = stack.top();
        int x = current.first;
        int y = current.second;

        QList<QPair<int, int>> neighbors;
        if (x > 2 && maze[y][x-2]) neighbors.append({x-2, y});
        if (x < mazeWidth-3 && maze[y][x+2]) neighbors.append({x+2, y});
        if (y > 2 && maze[y-2][x]) neighbors.append({x, y-2});
        if (y < mazeHeight-3 && maze[y+2][x]) neighbors.append({x, y+2});

        if (!neighbors.isEmpty()) {
            int idx = qrand() % neighbors.size();
            auto next = neighbors[idx];
            maze[(y+next.second)/2][(x+next.first)/2] = false;
            maze[next.second][next.first] = false;
            stack.push(next);
        } else {
            stack.pop();
        }
    }

    exitX = mazeWidth - 2;
    exitY = mazeHeight - 2;
    maze[exitY][exitX] = false;
}

bool MazeWidget::isValidMove(int x, int y)
{
    return (x >= 0 && x < mazeWidth && y >= 0 && y < mazeHeight) && !maze[y][x];
}

void MazeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(rect(), m_bgPixmap);
    } else {
        painter.fillRect(rect(), Qt::black);
    }

    int ox = (width() - mazeWidth * cellSize) / 2;
    int oy = (height() - mazeHeight * cellSize) / 2;

    for (int y = 0; y < mazeHeight; y++) {
        for (int x = 0; x < mazeWidth; x++) {
            int px = ox + x * cellSize;
            int py = oy + y * cellSize;
            if (maze[y][x]) {
                if (!m_wallPixmap.isNull()) {
                    painter.drawPixmap(px, py, cellSize, cellSize, m_wallPixmap);
                } else {
                    painter.fillRect(px, py, cellSize, cellSize, Qt::darkGray);
                }
            }
        }
    }

    // 玩家
    QPixmap *playerFrames = (m_lastDirectionX >= 0) ? m_playerPixmapR : m_playerPixmapL;
    if (!playerFrames[m_playerFrame].isNull()) {
        painter.drawPixmap(ox + playerX*cellSize + 2, oy + playerY*cellSize + 2,
                           cellSize-4, cellSize-4, playerFrames[m_playerFrame]);
    } else {
        painter.fillRect(ox + playerX*cellSize + 2, oy + playerY*cellSize + 2,
                         cellSize-4, cellSize-4, Qt::blue);
    }
    // 出口（宽度放大到1.1倍）
    int doorW = (cellSize - 4) * 1.1;
    int doorH = cellSize - 4;
    if (!m_doorPixmap.isNull()) {
        painter.drawPixmap(ox + exitX*cellSize + 2, oy + exitY*cellSize + 2,
                           doorW, doorH, m_doorPixmap);
    } else {
        painter.fillRect(ox + exitX*cellSize + 2, oy + exitY*cellSize + 2,
                         doorW, doorH, Qt::green);
    }

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

void MazeWidget::keyPressEvent(QKeyEvent *event)
{
    // ESC 暂停/继续
    if (event->key() == Qt::Key_Escape) {
        m_paused = !m_paused;
        if (m_paused) {
            animationTimer->stop();
            m_autoPathWasRunning = m_autoPathTimer->isActive();
            m_autoPathTimer->stop();
        } else {
            animationTimer->start(150);
            if (m_autoPathWasRunning) m_autoPathTimer->start(50);
        }
        emit gamePaused(m_paused);
        update();
        return;
    }

    if (m_paused) return;  // 暂停时忽略移动

    int nx = playerX, ny = playerY;

    switch (event->key()) {
    case Qt::Key_W: case Qt::Key_Up:    ny--; break;
    case Qt::Key_S: case Qt::Key_Down:  ny++; break;
    case Qt::Key_A: case Qt::Key_Left:  nx--; m_lastDirectionX = -1; break;
    case Qt::Key_D: case Qt::Key_Right: nx++; m_lastDirectionX = 1; break;
    default:
        QWidget::keyPressEvent(event);
        return;
    }

    if (isValidMove(nx, ny)) {
        playerX = nx;
        playerY = ny;
        update();

        if (playerX == exitX && playerY == exitY) {
            emit mazeCompleted();
        }
    }
}

void MazeWidget::animatePlayer()
{
    m_playerFrame = (m_playerFrame + 1) % 4;
    update();
}

void MazeWidget::onAutoPath()
{
    // 停止之前的自动寻路
    m_autoPathTimer->stop();
    m_autoPath.clear();
    m_autoPathIndex = 0;

    // BFS 寻路
    QVector<QVector<bool>> visited(mazeHeight, QVector<bool>(mazeWidth, false));
    QVector<QVector<QPair<int,int>>> parent(mazeHeight, QVector<QPair<int,int>>(mazeWidth, {-1,-1}));

    QQueue<QPair<int,int>> q;
    q.enqueue({playerX, playerY});
    visited[playerY][playerX] = true;

    int dx[] = {0, 0, -1, 1};
    int dy[] = {-1, 1, 0, 0};
    bool found = false;

    while (!q.isEmpty()) {
        auto cur = q.dequeue();
        int cx = cur.first, cy = cur.second;
        if (cx == exitX && cy == exitY) {
            found = true;
            break;
        }
        for (int i = 0; i < 4; i++) {
            int nx = cx + dx[i], ny = cy + dy[i];
            if (isValidMove(nx, ny) && !visited[ny][nx]) {
                visited[ny][nx] = true;
                parent[ny][nx] = cur;
                q.enqueue({nx, ny});
            }
        }
    }

    if (!found) return;

    // 从出口回溯到起点
    QVector<QPair<int,int>> path;
    int sx = exitX, sy = exitY;
    while (!(sx == playerX && sy == playerY)) {
        path.prepend({sx, sy});
        auto p = parent[sy][sx];
        sx = p.first;
        sy = p.second;
        if (sx == -1) break;
    }
    m_autoPath = path;
    m_autoPathIndex = 0;

    // 开始自动行走
    m_autoPathTimer->start(50);
}

void MazeWidget::autoStep()
{
    if (m_autoPathIndex >= m_autoPath.size()) {
        m_autoPathTimer->stop();
        return;
    }

    auto next = m_autoPath[m_autoPathIndex];
    int nx = next.first, ny = next.second;

    // 更新朝向
    if (nx > playerX) m_lastDirectionX = 1;
    else if (nx < playerX) m_lastDirectionX = -1;

    playerX = nx;
    playerY = ny;
    m_autoPathIndex++;
    update();

    // 到达出口
    if (playerX == exitX && playerY == exitY) {
        m_autoPathTimer->stop();
        emit mazeCompleted();
    }
}
