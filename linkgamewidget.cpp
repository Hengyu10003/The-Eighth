#include "linkgamewidget.h"
#include <QPainter>
#include <QTime>
#include <algorithm>
#include <QPushButton>

//Qt5.9需要自己写这个哈希函数
uint qHash(const QPoint &p, uint seed = 0)
{
    return qHash((static_cast<uint>(p.x()) << 16) |(static_cast<uint>(p.y()) & 0xFFFFU), seed);
}
//颜色表==============================================================================================================
static const QColor colorsTable[] = {
    QColor(231, 76, 60),     // 1 红
    QColor(41, 128, 185),    // 2 蓝
    QColor(39, 174, 96),     // 3 绿
    QColor(230, 126, 34),    // 4 橙
    QColor(142, 68, 173),    // 5 紫
    QColor(22, 160, 133),    // 6 青
    QColor(139, 90, 43),     // 7 棕
    QColor(219, 60, 155),    // 8 粉
    QColor(44, 62, 80),      // 9 深蓝
    QColor(241, 196, 15)     // 10 黄
};

//构造函数============================================================================================================
LinkGameWidget::LinkGameWidget(QWidget *parent, int difficulty, QSize size)
    : QWidget(parent)
    , difficulty(difficulty)
    , grid(nullptr)
    , colors(nullptr)
    , totalPairs(0)
    , matchedPairs(0)
    , m_isBuilding(false)
    , m_bgPixmap(LINE)
    , m_paused(false)
{
//锁定画面大小------------------------------------------------------------------------------------------------
    setFixedSize(size);//锁定窗格大小
    setFocusPolicy(Qt::StrongFocus);//强焦点
//不同难度界面初始化-----------------------------------------------------------------------------------------------
    //棋盘尺寸；数字范围
    switch (difficulty) {
    case 1: gridSize = 6;  numRange = 5;  break;
    case 2: gridSize = 10; numRange = 7;  break;
    case 3: gridSize = 12; numRange = 10; break;
    }
    //设置格子大小
    cellSize = qMin(width() / gridSize, height() / gridSize) * 0.9;
    //游戏初始化
    initGame();
//按钮----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    // 返回主菜单按钮
    QPushButton *backBtn = new QPushButton(this);
        //美工设计
    backBtn->setGeometry(1105, 0, 75, 45);
    backBtn->setText("返回");
    backBtn->setStyleSheet("QPushButton { background-color: rgba(200,200,200,200); border: 1px solid gray; border-radius: 5px; }"
                           "QPushButton:hover { background-color: rgba(255,255,255,230); }");
        //防止抢焦点
    backBtn->setFocusPolicy(Qt::NoFocus);
        //信息连接槽函数
    connect(backBtn, SIGNAL(clicked()), this, SIGNAL(returnToMenu()));
}
//析构函数
LinkGameWidget::~LinkGameWidget()
{
    if (grid) {
        for (int i = 0; i < gridSize; i++) {
            delete[] grid[i];
            delete[] colors[i];
        }
        delete[] grid;
        delete[] colors;
    }
}

// ===================== 棋盘生成（DFS 路径搜索）===========================================================================================================================================================
//初始化/重置游戏棋盘------------------------------------------------------------------------------------------------
void LinkGameWidget::initGame()
{
//内存分配-----------------------------------------------------------------------------------------------
    // 清理旧数组
    if (grid) {
        for (int i = 0; i < gridSize; i++) {
            delete[] grid[i];
            delete[] colors[i];
        }
        delete[] grid;
        delete[] colors;
    }
    // 分配新数组——二维数组
    grid = new int*[gridSize];
    colors = new int*[gridSize];
    for (int i = 0; i < gridSize; i++) {
        grid[i] = new int[gridSize];
        colors[i] = new int[gridSize];
        for (int j = 0; j < gridSize; j++) {
            grid[i][j] = 0;
            colors[i][j] = 0;
        }
    }
// 重置状态---------------------------------------------------------------------------------------------------------------------------------------------------
    totalPairs = numRange;  // 每个数字1对，总共 numRange 对
    matchedPairs = 0;//当前成功对数
    m_isBuilding = false;//是否正在连线
    m_buildPath.clear();//当前正在连线清空
    m_pairs.clear();//当前完成配对数清空
    m_connectionHistory.clear();//连线历史记录清空

//随机种子---------------------------------------------------------------------------------------------------------------------------------------------------
    qsrand((uint)QTime::currentTime().msec());
//用 DFS 生成棋盘
    if (!generateBoard()) {
        // 万一失败，清空重试一次
        for (int i = 0; i < gridSize; i++)
            for (int j = 0; j < gridSize; j++)
                grid[i][j] = 0;
        m_pairs.clear();
        qsrand((uint)QTime::currentTime().msec() + 1);// 随机种子+1，改变随机序列
        generateBoard();
    }

    // 为每个 pair 分配颜色索引（保证同数字不同 pair 颜色不同）
    for (int pi = 0; pi < m_pairs.size(); pi++) {
        // 每个 pair 一个唯一颜色索引
        const PairInfo &pair = m_pairs[pi];
        int colorIdx = pi;
        //起点位置和终点位置的颜色
        colors[pair.p1.y()][pair.p1.x()] = colorIdx;
        colors[pair.p2.y()][pair.p2.x()] = colorIdx;
    }

    update();
}

//生成数字连线棋盘-------------------------------------------------------------------------------------------------
bool LinkGameWidget::generateBoard()
{
    QSet<QPoint> reserved;              // 已经被占用的格子坐标
    m_pairs.clear();                    // 清掉上一局留下的配对记录

    // 把数字1~numRange包装一下，为了排序
    struct Item { int num; };
    QVector<Item> allPairs;             // 装所有数字的数组

    // 数字1到numRange，每个数字装进一个"盒子"
    for (int num = 1; num <= numRange; num++) {
        allPairs.append(Item{num});
    }

    // 把数字从大到小排好 → 大的先放，小的后放
    std::sort(allPairs.begin(), allPairs.end(),
              [](const Item &a, const Item &b) { return a.num > b.num; });

    // 遍历每个数字（从大到小）
    for (const Item &item : allPairs) {
        int num = item.num;             // 当前要放的是数字几
        bool placed = false;            // 标记：这个数字放成功了没？先假设没成功

        // 每个数字最多给120次随机尝试机会
        for (int attempt = 0; attempt < 120; attempt++) {

            // === 第一步：扫一遍棋盘，找出所有空位 ===
            QVector<QPoint> emptyCells;  // 存空位的数组
            for (int r = 0; r < gridSize; r++)
                for (int c = 0; c < gridSize; c++)
                    if (!reserved.contains(QPoint(c, r)))   // 这格没被占
                        emptyCells.append(QPoint(c, r));     // 加入空位列表

            // 空位不够2个 → 一对数字都塞不下 → 放弃这个数字
            if (emptyCells.size() < 2) break;

            // === 第二步：从空位里随机选一个当起点 ===
            QPoint start = emptyCells[qrand() % emptyCells.size()];

            // === 第三步：DFS 找路 ===
            // 从起点出发，走num步（数字越大走得越远），落到另一个空位上就算成功
            QVector<QPoint> foundPath;   // 用来装DFS找到的路径（起点→终点）
            QSet<QPoint> visited;        // 记录DFS搜过的格子，防止走回头路
            if (findBentPath(start, num, reserved, visited, foundPath)) {
                QPoint end = foundPath.last();   // 路径的最后一个格子就是终点

                // === 第四步：把数字填到起点和终点这两个格子上 ===
                grid[start.y()][start.x()] = num;
                grid[end.y()][end.x()] = num;

                // === 第五步：整条路径上的格子全部标记为"已被占" ===
                for (const QPoint &p : foundPath)
                    reserved.insert(p);

                // === 第六步：记下这一对数字的信息 ===
                PairInfo pi;
                pi.number = num;          // 数字几
                pi.p1 = start;            // 起点的坐标
                pi.p2 = end;              // 终点的坐标
                pi.connected = false;     // 还没被玩家连上
                m_pairs.append(pi);       // 存到配对的列表里

                placed = true;            // 标记：放成功了
                break;                    // 不用再试了，搞下一个数字
            }
            // 路径没找到 → 换一个起点重新试（最多120次）
        }
        if (!placed) return false;        // 120次都放不下 → 棋盘生成失败，重新来
    }
    return true;                         // 所有数字都放好了 → 棋盘生成成功
}


// 递归找路：从pos出发，走steps步，找个空位当终点
bool LinkGameWidget::findBentPath(const QPoint &pos, int steps,
                                  const QSet<QPoint> &reserved,   // 已被占的格子集合（只读）
                                  QSet<QPoint> &visited,          // 这轮搜索走过的格子（会修改）
                                  QVector<QPoint> &outPath) const // 找到的路径存到这里
{
// === 走到头了：步数走完，检查落点 ===
    if (steps == 0) {                                       // 要求的步数走完了
        if (!reserved.contains(pos)) {                      // 已被占用的格子不包含pos
            outPath.append(pos);                            // 把终点记到路径里
            return true;                                    // 告诉上一层：找到路了
        }
        return false;                                       // 落在别人地盘上 → 这条路不行
    }

// === 还没走完：标记当前位置，继续探索 ===
    visited.insert(pos);                                    // 告诉后面的搜索："这格我来过了，别走回头路"

// === 把上下左右四个方向打乱顺序 ===
    // 这样每次生成的棋盘路径都不一样，不会每次都往同一个方向走
    int order[4] = {0, 1, 2, 3};
    for (int i = 3; i > 0; i--) {
        int j = qrand() % (i + 1);
        int t = order[i]; order[i] = order[j]; order[j] = t;
    }

    // 上下左右对应的行变化和列变化
    static const int dr[] = {-1, 1, 0, 0};                  // 上、下、左、右 的行变化
    static const int dc[] = {0, 0, -1, 1};                  // 上、下、左、右 的列变化

    // 按打乱后的顺序，试四个方向
    for (int d = 0; d < 4; d++) {
        int nr = pos.y() + dr[order[d]];                    // 邻居的行坐标
        int nc = pos.x() + dc[order[d]];                    // 邻居的列坐标
        QPoint next(nc, nr);                                // 邻居的坐标

        // 检查：没出棋盘、没走过、没被占
        if (nc >= 0 && nc < gridSize && nr >= 0 && nr < gridSize &&
            !visited.contains(next) && !reserved.contains(next)) {
            //DFS核心：递归，从邻居出发继续走，步数减1
            if (findBentPath(next, steps - 1, reserved, visited, outPath)) {
                outPath.prepend(pos);                       // 把当前格子插到路径最前面（回溯时从终点倒着往回加）
                visited.remove(pos);                        // 把当前格子的标记清掉（让其他路径也能走这格）
                return true;                                // 告诉上一层：这条路走得通
            }
        }
    }

    // 四个方向都走不通 → 从这格出发没路可走
    visited.remove(pos);                                    // 清掉标记
    return false;                                           // 告诉上一层：这条路不行，换个方向试试
}

// ===================== 颜色 ===============================================================================================================================================================================================

QColor LinkGameWidget::getColor(int num) const
{
    if (num >= 1 && num <= 10) return colorsTable[num - 1];//根据颜色表中的颜色来选
    return Qt::gray;
}

// ===================== 辅助函数 ================================================================================================================================================================================================================
//计算点在棋盘中的位置
QPoint LinkGameWidget::cellAtPos(const QPoint &pos) const
{
    if (gridSize <= 0) return QPoint(-1, -1);
    int offsetX = (width() - gridSize * cellSize) / 2;      // 棋盘左边留多少空白
    int offsetY = (height() - gridSize * cellSize) / 2;     // 棋盘上边留多少空白
    int c = (pos.x() - offsetX) / cellSize;                 // 鼠标x → 棋盘第几列
    int r = (pos.y() - offsetY) / cellSize;                 // 鼠标y → 棋盘第几行
    if (r >= 0 && r < gridSize && c >= 0 && c < gridSize)   // 点在棋盘范围内
        return QPoint(c, r);                                // 返回格子坐标
    return QPoint(-1, -1);                                  // 点在棋盘外
}

// 两个格子是不是上下左右相邻
bool LinkGameWidget::isAdjacent(const QPoint &a, const QPoint &b) const
{
    return (qAbs(a.x() - b.x()) + qAbs(a.y() - b.y())) == 1;
}

// 检查是不是所有数字对都连上了，全连上就赢了
bool LinkGameWidget::checkWin()
{
    for (const PairInfo &pair : m_pairs)
        if (!pair.connected) return false;                  // 还有没连的，没赢
    return true;                                            // 全连上了，赢了
}

// 某个格子是属于第几对数字的？没连上的才算，连上了或压根不是数字格就返回-1
int LinkGameWidget::getPairIndex(const QPoint &cell) const
{
    // 遍历所有数字对
    for (int i = 0; i < m_pairs.size(); i++) {
        const PairInfo &pair = m_pairs[i];         // 拿出第 i 对

        // 条件1：这一对还没被玩家连上
        // 条件2：你点的格子正好是这一对的起点或终点
        if (!pair.connected && (pair.p1 == cell || pair.p2 == cell))
            return i;     // 找到了！返回这一对的编号
    }
    return -1;             // 所有对都查完了，没找到 → 返回 -1
}

// ===================== 鼠标事件 ================================================================================================================================================================================================================
//连线设置
void LinkGameWidget::mousePressEvent(QMouseEvent *event)
{
//暂停状态
    if (m_paused) return;
//点击位置
    //点的位置
    QPoint cell = cellAtPos(event->pos());//位置换算
    //点到棋盘外边了
    if (cell.x() < 0) return;
// 左键：点击数字开始连线
    if (event->button() == Qt::LeftButton)
        handleLeftClick(cell);
}
//暂停键
void LinkGameWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        m_paused = !m_paused;
        emit gamePaused(m_paused);
        update();
    }
}
//点击处理
void LinkGameWidget::handleLeftClick(const QPoint &cell)
{
    int v = grid[cell.y()][cell.x()];

    // ===== 不在建造模式 =====
    if (!m_isBuilding) {
        // 点击已连接的数字 → 撤销该 pair
        if (v < 0) {
            for (int i = 0; i < m_pairs.size(); i++) {
                PairInfo &pair = m_pairs[i];
                if (!pair.connected) continue;
                if (pair.p1 != cell && pair.p2 != cell) continue;

                // 恢复数字
                grid[pair.p1.y()][pair.p1.x()] = pair.number;
                grid[pair.p2.y()][pair.p2.x()] = pair.number;
                // 清空路径格子
                for (const QPoint &p : pair.path)
                    if (p != pair.p1 && p != pair.p2)
                        grid[p.y()][p.x()] = 0;
                pair.connected = false;
                pair.path.clear();
                m_connectionHistory.removeOne(i);
                update();
                return;
            }
            return;
        }

        // 点击空格 → 忽略
        if (v == 0) return;

        // 点击未连接的数字 → 开始建造
        int pairIdx = getPairIndex(cell);
        if (pairIdx < 0) return;

        m_isBuilding = true;
        m_buildStart = cell;
        m_buildPath.clear();
        m_buildPath.append(cell);
        update();
        return;
    }

    // ===== 正在建造模式 =====
    QPoint last = m_buildPath.last();

    // 点击自身 / 已在路径中 / 不相邻 → 取消建造
    if (cell == m_buildStart || m_buildPath.contains(cell) || !isAdjacent(cell, last)) {
        m_isBuilding = false;
        m_buildPath.clear();
        update();
        return;
    }

    int pathLen = m_buildPath.size();  // 当前路径格子数（含起点）
    int startVal = grid[m_buildStart.y()][m_buildStart.x()];

    if (v > 0) {
        // 点到数字格子
        if (v == startVal && pathLen == v) {
            // 步数正确 → 连接
            m_buildPath.append(cell);

            // 找这个 pair
            int pairIdx = -1;
            for (int i = 0; i < m_pairs.size(); i++) {
                PairInfo &pair = m_pairs[i];
                if (pair.connected || pair.number != v) continue;
                if ((pair.p1 == m_buildStart && pair.p2 == cell) ||
                    (pair.p2 == m_buildStart && pair.p1 == cell)) {
                    pairIdx = i;
                    break;
                }
            }

            if (pairIdx >= 0) {
                m_pairs[pairIdx].path = m_buildPath;
                connectPair(pairIdx);
            } else {
                // 两个数字同值但不属于同一对 → 取消
                m_isBuilding = false;
                m_buildPath.clear();
                update();
            }
        } else {
            // 步数不对 / 不同数字 → 取消
            m_isBuilding = false;
            m_buildPath.clear();
            update();
        }
        return;
    }

    // 点到已连的格子 → 取消
    if (v < 0) {
        m_isBuilding = false;
        m_buildPath.clear();
        update();
        return;
    }

    // 空格 → 延伸路径（最多添加到 pathLen < startVal 个中间格）
    if (pathLen < startVal) {
        m_buildPath.append(cell);
        update();
    }
}

// 玩家成功连上一对数字后，执行收尾工作
void LinkGameWidget::connectPair(int idx)
{
    // 检查：编号不对就直接退出
    if (idx < 0 || idx >= m_pairs.size()) return;

    PairInfo &pair = m_pairs[idx];       // 取出连上的这一对

    // 把这一对的整条路径填上负数的数字
    // 比如数字3的路径，所有格子都填 -3，表示这条路已经被占了
    for (const QPoint &p : pair.path) {
        grid[p.y()][p.x()] = -pair.number;   // 填负数，和没被连的数字区分开
        colors[p.y()][p.x()] = idx;           // 路径上的格子也涂上和端点一样的颜色
    }

    pair.connected = true;                 // 标记：这一对连上了
    m_connectionHistory.append(idx);       // 记下连线的顺序（方便撤销用）

    m_isBuilding = false;                  // 取消"正在拖线"状态
    m_buildPath.clear();                   // 清掉拖线时画的临时路径

    matchedPairs++;                        // 已连成对的数量+1

    update();                              // 刷新画面，显示连好的线

    // 检查是不是所有对都连上了
    if (checkWin())
        emit gameWon();                    // 全连完了 → 告诉外面：这关过了！
}

// ===================== 绘图 =========================================================================================================================================================================================================================
void LinkGameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);                           // 拿一支笔，准备在当前控件上画画
    painter.setRenderHint(QPainter::Antialiasing, true); // 打开抗锯齿，画出来的线边缘更平滑，不会出现锯齿

// 背景
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(rect(), m_bgPixmap);
    } else {
        painter.fillRect(rect(), QColor(236, 240, 241));
    }
//棋盘
    // 棋盘没初始化好 → 不画了
    if (gridSize <= 0) return;

    // 计算棋盘在窗口左边和上边空出多少像素（为了让棋盘居中）
    int offsetX = (width() - gridSize * cellSize) / 2;
    int offsetY = (height() - gridSize * cellSize) / 2;

    //棋盘位置
    auto cellRect = [&](int r, int c) {
        return QRect(
            offsetX + c * cellSize + 1,        // 格子左边位置 = 偏移 + 列号×格宽 + 缩进1像素
            offsetY + r * cellSize + 1,        // 格子上边位置 = 偏移 + 行号×格高 + 缩进1像素
            cellSize - 2, cellSize - 2         // 格子宽高比格大小少2像素（留边距，看起来有间隙）
        );
    };

//绘制每个格子
    for (int i = 0; i < gridSize; i++) {          // i = 行号（从上往下）
        for (int j = 0; j < gridSize; j++) {      // j = 列号（从左往右）
            QRect rct = cellRect(i, j);           // 算出第i行第j列的格子画在屏幕上的矩形位置
            int v = grid[i][j];                   // 格子里的值：0=空, 正数=未连的数字, 负数=已连的路径
            int cidx = colors[i][j];              // 这个格子用的颜色编号

            // === 情况1：空格子 ===
            if (v == 0) {
                // 只画一个白色方框，不填充
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(Qt::white, 1));
                painter.drawRect(rct);
            }
            // === 情况2：还没连上的数字格子 ===
            else if (v > 0) {
                int n = v;
                // 如果有颜色编号就用HSV算一个颜色，没有就用默认颜色
                QColor baseColor = (cidx >= 0) ?
                    QColor::fromHsv((cidx * 360 / qMax(1, (int)m_pairs.size())) % 360, 200, 255) :
                    getColor(n);

                // 画一个深色边框的圆角矩形
                painter.setPen(QPen(baseColor.darker(110), 2));
                painter.setBrush(Qt::NoBrush);                     // 透明不填充
                painter.drawRoundedRect(rct.adjusted(1, 1, -1, -1), 4, 4);

                // 在格子中间写上数字（比如 "3"、"5"）
                QFont f;
                f.setPointSize(qMax(8, qMin(cellSize, cellSize) / 2));  // 字号根据格子大小调
                f.setBold(true);
                painter.setFont(f);
                painter.setPen(baseColor);                         // 数字颜色和边框一致
                painter.drawText(rct, Qt::AlignCenter, QString::number(n));
            }
            // === 情况3：已经连好的路径格子 ===
            else {
                int n = -v;                                        // 负数转回正数，知道是数字几
                // 算颜色
                QColor baseColor = (cidx >= 0 && cidx < m_pairs.size()) ?
                    QColor::fromHsv((cidx * 360 / qMax(1, (int)m_pairs.size())) % 360, 200, 255) :
                    getColor(n);

                // 整格填满颜色（实心）
                painter.setPen(Qt::NoPen);
                painter.fillRect(rct, baseColor);
                QColor darker = baseColor.darker(130);              // 颜色加深一点
                painter.setPen(QPen(darker, 1));
                painter.drawRect(rct);                              // 画个深色细边框

                // 检查这格是不是这一对的起点或终点
                bool isEndpoint = false;
                for (const PairInfo &pair : m_pairs) {
                    if (pair.connected) {                           // 只检查已连上的对
                        if ((pair.p1.x() == j && pair.p1.y() == i) ||   // 这格是起点？
                            (pair.p2.x() == j && pair.p2.y() == i)) {   // 这格是终点？
                            isEndpoint = true;
                            break;
                        }
                    }
                }

                // 只有端点才写数字，中间的路径格子只填色不写字
                if (isEndpoint) {
                    QFont f;
                    f.setPointSize(qMax(8, qMin(cellSize, cellSize) / 2));
                    f.setBold(true);
                    painter.setFont(f);
                    painter.setPen(Qt::white);                       // 端点的数字用白色
                    painter.drawText(rct, Qt::AlignCenter, QString::number(n));
                }
            }
        }
    }
// 绘制建造路径
    if (m_isBuilding && !m_buildPath.isEmpty()) {
        // 获取起点格子里的数字（取绝对值，因为连上后可能变成负数）
        int n = qAbs(grid[m_buildStart.y()][m_buildStart.x()]);
        if (n == 0) n = 1;                        // 保底：如果取出来是0就用1

        // 获取起点格子的颜色编号
        int cidx = colors[m_buildStart.y()][m_buildStart.x()];
        // 算路径的颜色（跟起点数字一样的颜色）
        QColor pathColor = (cidx >= 0) ?
            QColor::fromHsv((cidx * 360 / qMax(1, (int)m_pairs.size())) % 360, 255, 255) :
            getColor(n);

        // 在路径格子中间画半透明小方块（鼠标拖到哪就亮到哪）
        QColor fill = pathColor;
        fill.setAlpha(80);                               // 透明度设80，半透明效果
        for (int k = 1; k < m_buildPath.size(); k++) {   // 从第1格开始（第0格是起点）
            QPoint p = m_buildPath[k];
            painter.fillRect(cellRect(p.y(), p.x()).adjusted(3, 3, -3, -3), fill);
        }

        // 在路径格子之间画虚线，把走过的格子串起来
        painter.setPen(QPen(pathColor, 3, Qt::DashLine));   // 颜色和起点一致，虚线
        for (int k = 1; k < m_buildPath.size(); k++) {
            // 计算上一格的中心点
            QPointF a(offsetX + m_buildPath[k-1].x() * cellSize + cellSize / 2.0,
                      offsetY + m_buildPath[k-1].y() * cellSize + cellSize / 2.0);
            // 计算当前格的中心点
            QPointF b(offsetX + m_buildPath[k].x() * cellSize + cellSize / 2.0,
                      offsetY + m_buildPath[k].y() * cellSize + cellSize / 2.0);
            painter.drawLine(a, b);                         // 画一条线连起来
        }

        // 起点加一个黄色高亮边框，提示玩家"你是从这里开始拖的"
        painter.setPen(QPen(QColor(241, 196, 15), 3));      // 金色边框
        painter.drawRoundedRect(
            cellRect(m_buildStart.y(), m_buildStart.x()).adjusted(2, 2, -2, -2), 4, 4);
    }

// 暂停遮罩绘制
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
