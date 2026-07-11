#include "mazewidget.h"
#include <QPainter>
#include <QTime>
#include <QStack>
#include <QPair>
#include <QDebug>
#include <QKeyEvent>

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

    // 这个 timer 你没 connect，没用但不影响，我先保留
    animationTimer = new QTimer(this);
    // animationTimer->start(100); // 不需要，注释掉更干净

    // 关键修复：打开时自动获取焦点，保证按键立刻能用
    setFocus();
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
    painter.fillRect(rect(), Qt::black);

    int ox = (width() - mazeWidth * cellSize) / 2;
    int oy = (height() - mazeHeight * cellSize) / 2;

    for (int y = 0; y < mazeHeight; y++) {
        for (int x = 0; x < mazeWidth; x++) {
            int px = ox + x * cellSize;
            int py = oy + y * cellSize;
            painter.fillRect(px, py, cellSize, cellSize,
                             maze[y][x] ? Qt::darkGray : Qt::white);
        }
    }

    // 玩家
    painter.fillRect(ox + playerX*cellSize + 2, oy + playerY*cellSize + 2,
                     cellSize-4, cellSize-4, Qt::blue);
    // 出口
    painter.fillRect(ox + exitX*cellSize + 2, oy + exitY*cellSize + 2,
                     cellSize-4, cellSize-4, Qt::green);
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
