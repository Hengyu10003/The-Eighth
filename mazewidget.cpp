#include "mazewidget.h"
#include <QPainter>
#include <QTime>
#include <QStack>
#include <QPair>
#include <QDebug>
#include <QKeyEvent>
#include <QPushButton>

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
    m_playerPixmap[0].load(TUAN1_r);
    m_playerPixmap[1].load(TUAN2_r);
    m_playerPixmap[2].load(TUAN3_r);
    m_playerPixmap[3].load(TUAN4_r);
    m_playerFrame = 0;
    m_doorPixmap.load(DOOR);
    m_wallPixmap.load(WALL);
    m_bgPixmap.load(MIGONG);

    animationTimer = new QTimer(this);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(animatePlayer()));
    animationTimer->start(150);

    // 关键修复：打开时自动获取焦点，保证按键立刻能用
    setFocus();

    // 返回主菜单按钮 (1150, 0 位置)
    QPushButton *backBtn = new QPushButton(this);
    backBtn->setGeometry(1050, 0, 100, 60);
    backBtn->setText("返回");
    backBtn->setStyleSheet("QPushButton { border-image: url(" WALL "); color: black; font-weight: bold; }"
                           "QPushButton:hover { background: rgba(255,255,255,100); }");
    backBtn->setFocusPolicy(Qt::NoFocus);
    connect(backBtn, SIGNAL(clicked()), this, SIGNAL(returnToMenu()));
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
    if (!m_playerPixmap[m_playerFrame].isNull()) {
        painter.drawPixmap(ox + playerX*cellSize + 2, oy + playerY*cellSize + 2,
                           cellSize-4, cellSize-4, m_playerPixmap[m_playerFrame]);
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
}

void MazeWidget::keyPressEvent(QKeyEvent *event)
{
    int nx = playerX, ny = playerY;

    switch (event->key()) {
    case Qt::Key_W: case Qt::Key_Up:    ny--; break;
    case Qt::Key_S: case Qt::Key_Down:  ny++; break;
    case Qt::Key_A: case Qt::Key_Left:  nx--; break;
    case Qt::Key_D: case Qt::Key_Right: nx++; break;
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
