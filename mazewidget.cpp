#include "mazewidget.h"
#include <QPainter>
#include <QTime>
#include <QStack>
#include <QPair>
#include <QDebug>
#include <QKeyEvent>
#include <QPushButton>
#include <QQueue>

//初始化界面设置========================================================================================================
MazeWidget::MazeWidget(QWidget *parent, int level, QSize size)
    : QWidget(parent)
    , level(level)
    , playerX(1)//初始化玩家的初始位置
    , playerY(1)
    , maze(nullptr)
{
//锁定画面
    setFixedSize(size);             //锁定大小（边界）
    setFocusPolicy(Qt::StrongFocus);//强焦点：鼠标、Tab、代码调用强焦点
//设置迷宫
    //设置长宽格子数——根据难度
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
        break;
    }
    //设置单个格子大小
    cellSize = qMin(width() / mazeWidth, height() / mazeHeight);//取较小的那一个
    //生成迷宫地图
    generateMaze();

//美工界面
//玩家
    // 加载玩家动画
    //（向右）
    m_playerPixmapR[0].load(TUAN1_r);
    m_playerPixmapR[1].load(TUAN2_r);
    m_playerPixmapR[2].load(TUAN3_r);
    m_playerPixmapR[3].load(TUAN4_r);
    //（向左）
    m_playerPixmapL[0].load(TUAN1_l);
    m_playerPixmapL[1].load(TUAN2_l);
    m_playerPixmapL[2].load(TUAN3_l);
    m_playerPixmapL[3].load(TUAN4_l);
    //开始帧数和朝向
    m_playerFrame = 0;//开始帧
    m_lastDirectionX = 1;//朝向
    //玩家动画定时器
    animationTimer = new QTimer(this);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(animatePlayer()));
    animationTimer->start(150);

    //加载图片
    m_doorPixmap.load(DOOR);//门
    m_wallPixmap.load(WALL);//墙
    m_bgPixmap.load(MIGONG);//背景

    //打开时自动获取焦点，保证按键立刻能用
    setFocus();

//按钮
    // 返回主菜单按钮
    QPushButton *backBtn = new QPushButton(this);
        //美工
    backBtn->setGeometry(1050, 0, 100, 60);//设置控件的位置和大小
    backBtn->setText("返回");//设置控件的文字
    backBtn->setStyleSheet("QPushButton { border-image: url(" WALL "); color: black; font-weight: bold; }"
                           "QPushButton:hover { background: rgba(255,255,255,100); }");
        //转焦点，防误触
    backBtn->setFocusPolicy(Qt::NoFocus);
        //连接信息-槽函数
    connect(backBtn, SIGNAL(clicked()), this, SIGNAL(returnToMenu()));

    // 自动寻路按钮
    QPushButton *autoBtn = new QPushButton(this);
        //美工
    autoBtn->setGeometry(1050, 65, 100, 60);
    autoBtn->setText("自动");
    autoBtn->setStyleSheet("QPushButton { border-image: url(" WALL "); color: black; font-weight: bold; }"
                           "QPushButton:hover { background: rgba(255,255,255,100); }");
        //防止抢占焦点
    autoBtn->setFocusPolicy(Qt::NoFocus);
        //执行自动寻路
    connect(autoBtn, SIGNAL(clicked()), this, SLOT(onAutoPath()));

    // 自动寻路定时器
    m_autoPathTimer = new QTimer(this);
    connect(m_autoPathTimer, SIGNAL(timeout()), this, SLOT(autoStep()));
    m_autoPathIndex = 0;
    m_paused = false;
    m_autoPathWasRunning = false;
}
//析构函数
MazeWidget::~MazeWidget()
{
    //释放maze二维数组占用内存
    if (maze) {
        for (int i = 0; i < mazeHeight; i++)
            delete[] maze[i];
        delete[] maze;
    }
}

//生成迷宫地图（DFS）1==================================================================================================================================
void MazeWidget::generateMaze()
{
//分配空间
    //清理旧迷宫
    if (maze) {
        for (int i = 0; i < mazeHeight; i++)
            delete[] maze[i];
        delete[] maze;
    }

    //分配新迷宫数组——二维数组
    maze = new bool*[mazeHeight];
    for (int i = 0; i < mazeHeight; i++) {
        maze[i] = new bool[mazeWidth];
        for (int j = 0; j < mazeWidth; j++)
            maze[i][j] = true;
    }
//DFS生成迷宫（深度优先生成树）
    qsrand(QTime::currentTime().msec());//随机种子

    QStack<QPair<int, int>> stack;//设置栈（数据类型是一对一类型）
    maze[1][1] = false;
    stack.push({1, 1});//设置入口

    while (!stack.isEmpty()) {                                  // 循环直到所有格子都被访问过
            //从入口位置开始访问v0
            auto current = stack.top();                         // 取栈顶元素（当前所在格子）
            int x = current.first;                              // 当前格子x坐标（列）
            int y = current.second;                             // 当前格子y坐标（行）

            // 收集所有可打通的邻居（隔一个格子，间距为2，确保墙厚一格）（奇数行奇数列作为结点）
            QList<QPair<int, int>> neighbors;
            if (x > 2 && maze[y][x-2]) neighbors.append({x-2, y});     // 左邻（x-2）
            if (x < mazeWidth-3 && maze[y][x+2]) neighbors.append({x+2, y}); // 右邻（x+2）
            if (y > 2 && maze[y-2][x]) neighbors.append({x, y-2});     // 上邻（y-2）
            if (y < mazeHeight-3 && maze[y+2][x]) neighbors.append({x, y+2}); // 下邻（y+2）

            if (!neighbors.isEmpty()) {                         // 有未访问的邻居
                int idx = qrand() % neighbors.size();           // 随机选一个邻居
                auto next = neighbors[idx];                     // 确定要打通的邻居格子
                maze[(y+next.second)/2][(x+next.first)/2] = false; // 打通中间的墙（两格子之间的间隔）
                maze[next.second][next.first] = false;           // 打通邻居格子本身
                stack.push(next);                                // 邻居入栈，继续从此处挖路
            } else {                                             // 无可用邻居，回溯
                stack.pop();                                     // 退回上一个格子
            }
        }

    //设置出口
    exitX = mazeWidth - 2;
    exitY = mazeHeight - 2;
    maze[exitY][exitX] = false;
}

//检测某个格子是否能走 5
bool MazeWidget::isValidMove(int x, int y)
{
    return (x >= 0 && x < mazeWidth && y >= 0 && y < mazeHeight) && !maze[y][x];
}

//绘制画面 3==============================================================================================================================================================
void MazeWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);//防止没用到参数编译器报错
    QPainter painter(this);//当前控件创建画笔

    //绘制背景=============================================================================================================================================================
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(rect(), m_bgPixmap);
    } else {
        painter.fillRect(rect(), Qt::black);
    }

    //迷宫居中============================================================================================================================================================
    int ox = (width() - mazeWidth * cellSize) / 2;
    int oy = (height() - mazeHeight * cellSize) / 2;

    //墙壁============================================================================================================================================================
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

    //玩家============================================================================================================================================================
    QPixmap *playerFrames = (m_lastDirectionX >= 0) ? m_playerPixmapR : m_playerPixmapL;
    if (!playerFrames[m_playerFrame].isNull()) {//图片
        painter.drawPixmap(ox + playerX*cellSize + 2, oy + playerY*cellSize + 2,
                           cellSize-4, cellSize-4, playerFrames[m_playerFrame]);
    } else {//蓝块
        painter.fillRect(ox + playerX*cellSize + 2, oy + playerY*cellSize + 2,
                         cellSize-4, cellSize-4, Qt::blue);
    }
    // 出口（宽度放大到1.1倍）============================================================================================================================================================
    int doorW = (cellSize - 4) * 1.1;
    int doorH = cellSize - 4;
    if (!m_doorPixmap.isNull()) {
        painter.drawPixmap(ox + exitX*cellSize + 2, oy + exitY*cellSize + 2,
                           doorW, doorH, m_doorPixmap);
    } else {
        painter.fillRect(ox + exitX*cellSize + 2, oy + exitY*cellSize + 2,
                         doorW, doorH, Qt::green);
    }

    // 暂停遮罩============================================================================================================================================================
    if (m_paused) {
        painter.fillRect(rect(), QColor(0, 0, 0, 160));//半透明黑底
        painter.setPen(Qt::white);//设置画笔颜色
        //设置字体
        QFont f = painter.font();//默认字体
        f.setPointSize(36);//设置字号
        f.setBold(true);//加粗
        painter.setFont(f);//引用这个字体
        //设置文本
        painter.drawText(rect(), Qt::AlignCenter, "已暂停\n按 ESC 继续");//水平垂直居中
    }
}

//处理键盘按下事件 4==============================================================================================================================================================
void MazeWidget::keyPressEvent(QKeyEvent *event)
{
    //=== ESC键：暂停/继续切换 ===
    if (event->key() == Qt::Key_Escape) {
        m_paused = !m_paused;                               // 翻转暂停状态
        if (m_paused) {                                     // 暂停
            animationTimer->stop();                         // 停止玩家动画（暂停帧切换）
            m_autoPathWasRunning = m_autoPathTimer->isActive(); // 记录自动寻路是否在运行
            m_autoPathTimer->stop();                        // 停止自动寻路
        } else {                                            // 恢复
            animationTimer->start(150);                     // 重启玩家动画（每150ms切换一帧）
            if (m_autoPathWasRunning) m_autoPathTimer->start(50); // 恢复自动寻路（每50ms走一步）
        }
        emit gamePaused(m_paused);                          // 发射暂停信号，通知MainWindow停止/恢复计时
        update();                                           // 触发重绘，显示/隐藏暂停遮罩
        return;
    }

    //=== 方向键/WASD：移动玩家 ===
    if (m_paused) return;                                   // 暂停状态下忽略所有移动按键

    int nx = playerX, ny = playerY;                         // 先计算目标位置，不直接修改玩家坐标

    switch (event->key()) {
    case Qt::Key_W: case Qt::Key_Up:    ny--; break;        // 上移（行号减1）
    case Qt::Key_S: case Qt::Key_Down:  ny++; break;        // 下移（行号加1）
    case Qt::Key_A: case Qt::Key_Left:  nx--; m_lastDirectionX = -1; break;  // 左移，记录朝左
    case Qt::Key_D: case Qt::Key_Right: nx++; m_lastDirectionX = 1; break;   // 右移，记录朝右
    default:
        QWidget::keyPressEvent(event);                      // 其他按键交给父类处理
        return;
    }

    if (isValidMove(nx, ny)) {                              // 检查目标位置是否可通行（不越界且不是墙）
        playerX = nx;                                       // 更新玩家列坐标
        playerY = ny;                                       // 更新玩家行坐标
        update();                                           // 触发重绘，显示玩家新位置
        // 到达出口
        if (playerX == exitX && playerY == exitY)
        {
            emit mazeCompleted();                           // 发射"迷宫通关"信号，通知GameController
        }
    }
}

//玩家动画切换 2==============================================================================================================================================================
void MazeWidget::animatePlayer()
{
    m_playerFrame = (m_playerFrame + 1) % 4;
    update();
}

//自动寻路==============================================================================================================================================================
// 自动寻路（BFS广度优先搜索）
void MazeWidget::onAutoPath()
{
    //=== 第一步：清理上次的自动寻路 ===
    m_autoPathTimer->stop();                                // 停止正在运行的自动寻路定时器
    m_autoPath.clear();                                     // 清空旧路径
    m_autoPathIndex = 0;                                    // 路径索引归零

    //=== 第二步：BFS从玩家当前位置搜索到出口的最短路径 ===
    // visited：标记格子是否已访问，防止重复搜索
    QVector<QVector<bool>> visited(mazeHeight, QVector<bool>(mazeWidth, false));
    // parent：记录每个格子的"前驱格子"，用于最后回溯整条路径
    QVector<QVector<QPair<int,int>>> parent(mazeHeight, QVector<QPair<int,int>>(mazeWidth, {-1,-1}));

    //队列
    QQueue<QPair<int,int>> q;                               // BFS队列
    q.enqueue({playerX, playerY});                          // 起点入队（玩家当前位置）
    visited[playerY][playerX] = true;                       // 起点标记为已访问

    int dx[] = {0, 0, -1, 1};                               // 四个方向的x偏移：不动、不动、左、右
    int dy[] = {-1, 1, 0, 0};                               // 四个方向的y偏移：上、下、不动、不动
    bool found = false;                                     // 是否找到出口

    while (!q.isEmpty()) {                                  // 队列不为空就继续搜索
        auto cur = q.dequeue();                             // 取出队列最前面的格子
        int cx = cur.first, cy = cur.second;                // 当前格子的x(列)、y(行)坐标
        if (cx == exitX && cy == exitY) {                   // 到达出口
            found = true;
            break;
        }
        for (int i = 0; i < 4; i++) {                       // 遍历上、下、左、右四个方向
            int nx = cx + dx[i], ny = cy + dy[i];           // 计算邻居格子的坐标
            if (isValidMove(nx, ny) && !visited[ny][nx]) {  // 邻居可通行且未被访问
                visited[ny][nx] = true;                     // 标记为已访问
                parent[ny][nx] = cur;                       // 记录邻居的前驱是当前格子
                q.enqueue({nx, ny});                        // 邻居入队，后续继续搜索
            }
        }
    }

    if (!found) return;                                     // 没找到出口（理论上不可能），直接返回

    //=== 第三步：从出口开始，通过parent数组回溯到起点，重建完整路径 ===
    QVector<QPair<int,int>> path;                           // 存储从起点到出口的路径
    int sx = exitX, sy = exitY;                             // 从出口开始回溯
    while (!(sx == playerX && sy == playerY)) {             // 还没回溯到起点就继续
        path.prepend({sx, sy});//将当前格子插入path
        auto p = parent[sy][sx];// 获取当前格子的前驱
        sx = p.first;// 往前回溯一格的x坐标
        sy = p.second;// 往前回溯一格的y坐标
        if (sx == -1) break;// 异常情况：前驱不存在，终止
    }
    m_autoPath = path;                                      // 保存计算出的路径
    m_autoPathIndex = 0;                                    // 路径索引归零，从第一步开始走

    //=== 第四步：启动行走定时器，每50ms走一步 ===
    m_autoPathTimer->start(50);
}

// 自动行走一步（由 autoPathTimer 每50ms触发一次）
void MazeWidget::autoStep()
{
    if (m_autoPathIndex >= m_autoPath.size()) {// 路径已走完（正常情况下不会发生，因为到达出口会提前停止）
        m_autoPathTimer->stop();// 停止自动寻路定时器
        return;
    }

    auto next = m_autoPath[m_autoPathIndex];                // 取出路径中下一步的目标格子
    int nx = next.first, ny = next.second;                  // 目标格子的x(列)、y(行)坐标

    // 根据移动方向更新玩家朝向（用于动画显示）
    if (nx > playerX) m_lastDirectionX = 1;                 // 向右走
    else if (nx < playerX) m_lastDirectionX = -1;           // 向左走

    playerX = nx;                                           // 更新玩家列坐标
    playerY = ny;                                           // 更新玩家行坐标
    m_autoPathIndex++;                                      // 路径索引前进到下一步
    update();                                               // 触发重绘，显示玩家新位置

    // 到达出口
    if (playerX == exitX && playerY == exitY) {             // 玩家到达迷宫出口
        m_autoPathTimer->stop();                            // 停止自动寻路定时器
        emit mazeCompleted();                               // 发射"迷宫通关"信号
    }
}
