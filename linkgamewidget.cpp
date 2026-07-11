#include "linkgamewidget.h"
#include <QPainter>
#include <QTime>
#include <algorithm>
#include <QPushButton>

// 让 QSet<QPoint> 能正常工作（Qt 5.9 需要）
uint qHash(const QPoint &p, uint seed = 0)
{
    return qHash((static_cast<uint>(p.x()) << 16) |
                 (static_cast<uint>(p.y()) & 0xFFFFU), seed);
}

// ===================== 颜色表（10个数字各一种）=====================
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

static const QColor lightColorsTable[] = {
    QColor(247, 169, 156),
    QColor(139, 194, 231),
    QColor(131, 219, 166),
    QColor(247, 189, 131),
    QColor(195, 155, 211),
    QColor(129, 207, 190),
    QColor(186, 150, 115),
    QColor(243, 158, 210),
    QColor(127, 140, 141),
    QColor(249, 226, 132)
};

// ===================== 构造 / 析构 =====================

LinkGameWidget::LinkGameWidget(QWidget *parent, int difficulty, QSize size)
    : QWidget(parent)
    , difficulty(difficulty)
    , grid(nullptr)
    , colors(nullptr)
    , totalPairs(0)
    , matchedPairs(0)
    , m_isBuilding(false)
    , m_bgPixmap(LINE)
{
    setFixedSize(size);
    setFocusPolicy(Qt::StrongFocus);

    switch (difficulty) {
    case 1: gridSize = 6;  numRange = 5;  break;
    case 2: gridSize = 10; numRange = 7;  break;
    case 3: gridSize = 12; numRange = 10; break;
    }

    cellSize = qMin(width() / gridSize, height() / gridSize) * 0.9;
    initGame();

    // 返回主菜单按钮
    QPushButton *backBtn = new QPushButton(this);
    backBtn->setGeometry(1105, 0, 75, 45);
    backBtn->setText("返回");
    backBtn->setStyleSheet("QPushButton { background-color: rgba(200,200,200,200); border: 1px solid gray; border-radius: 5px; }"
                           "QPushButton:hover { background-color: rgba(255,255,255,230); }");
    backBtn->setFocusPolicy(Qt::NoFocus);
    connect(backBtn, SIGNAL(clicked()), this, SIGNAL(returnToMenu()));
}

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

// ===================== 公开接口 =====================

void LinkGameWidget::setDifficulty(int difficulty)
{
    this->difficulty = difficulty;
    switch (difficulty) {
    case 1: gridSize = 6;  numRange = 5;  break;
    case 2: gridSize = 10; numRange = 7;  break;
    case 3: gridSize = 12; numRange = 10; break;
    }
    cellSize = qMin(width() / gridSize, height() / gridSize);
}

void LinkGameWidget::reset()
{
    initGame();
}

// ===================== 棋盘生成（DFS 路径搜索）=====================

void LinkGameWidget::initGame()
{
    // 清理旧数组
    if (grid) {
        for (int i = 0; i < gridSize; i++) {
            delete[] grid[i];
            delete[] colors[i];
        }
        delete[] grid;
        delete[] colors;
    }

    // 分配新数组
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

    // 重置状态
    totalPairs = numRange;  // 每个数字1对，总共 numRange 对
    matchedPairs = 0;
    m_isBuilding = false;
    m_buildPath.clear();
    m_pairs.clear();
    m_connectionHistory.clear();

    qsrand((uint)QTime::currentTime().msec());

    // 用 DFS 生成棋盘
    if (!generateBoard()) {
        // 万一失败，清空重试一次
        for (int i = 0; i < gridSize; i++)
            for (int j = 0; j < gridSize; j++)
                grid[i][j] = 0;
        m_pairs.clear();
        qsrand((uint)QTime::currentTime().msec() + 1);
        generateBoard();
    }

    // 为每个 pair 分配颜色索引（保证同数字不同 pair 颜色不同）
    for (int pi = 0; pi < m_pairs.size(); pi++) {
        const PairInfo &pair = m_pairs[pi];
        int colorIdx = pi;  // 每个 pair 一个唯一颜色索引
        colors[pair.p1.y()][pair.p1.x()] = colorIdx;
        colors[pair.p2.y()][pair.p2.x()] = colorIdx;
    }

    update();
}

bool LinkGameWidget::generateBoard()
{
    QSet<QPoint> reserved;
    m_pairs.clear();

    struct Item { int num; };
    QVector<Item> allPairs;

    // 每个数字恰好1对
    for (int num = 1; num <= numRange; num++) {
        allPairs.append(Item{num});
    }

    // 从大到小排序
    std::sort(allPairs.begin(), allPairs.end(),
              [](const Item &a, const Item &b) { return a.num > b.num; });

    for (const Item &item : allPairs) {
        int num = item.num;
        bool placed = false;

        for (int attempt = 0; attempt < 120; attempt++) {
            // 收集所有空单元格
            QVector<QPoint> emptyCells;
            for (int r = 0; r < gridSize; r++)
                for (int c = 0; c < gridSize; c++)
                    if (!reserved.contains(QPoint(c, r)))
                        emptyCells.append(QPoint(c, r));

            if (emptyCells.size() < 2) break;

            // 随机选起点
            QPoint start = emptyCells[qrand() % emptyCells.size()];

            // DFS 找一条长度为 num 步的路径到另一个空单元格
            QVector<QPoint> foundPath;
            QSet<QPoint> visited;
            if (findBentPath(start, num, reserved, visited, foundPath)) {
                QPoint end = foundPath.last();

                // 放置数字
                grid[start.y()][start.x()] = num;
                grid[end.y()][end.x()] = num;

                // 路径上的所有格子标记为已占用
                for (const QPoint &p : foundPath)
                    reserved.insert(p);

                PairInfo pi;
                pi.number = num;
                pi.p1 = start;
                pi.p2 = end;
                pi.connected = false;
                m_pairs.append(pi);

                placed = true;
                break;
            }
        }
        if (!placed) return false;
    }
    return true;
}

// DFS：从 pos 出发，走 steps 步，到达一个未被保留的空格子
// 路径长度 = steps 条边 = steps+1 个格子（含起点和终点）
bool LinkGameWidget::findBentPath(const QPoint &pos, int steps,
                                  const QSet<QPoint> &reserved,
                                  QSet<QPoint> &visited,
                                  QVector<QPoint> &outPath) const
{
    if (steps == 0) {
        if (!reserved.contains(pos)) {
            outPath.append(pos);
            return true;
        }
        return false;
    }

    visited.insert(pos);

    // 随机方向顺序
    int order[4] = {0, 1, 2, 3};
    for (int i = 3; i > 0; i--) {
        int j = qrand() % (i + 1);
        int t = order[i]; order[i] = order[j]; order[j] = t;
    }

    static const int dr[] = {-1, 1, 0, 0};
    static const int dc[] = {0, 0, -1, 1};

    for (int d = 0; d < 4; d++) {
        int nr = pos.y() + dr[order[d]];
        int nc = pos.x() + dc[order[d]];
        QPoint next(nc, nr);

        if (nc >= 0 && nc < gridSize && nr >= 0 && nr < gridSize &&
            !visited.contains(next) && !reserved.contains(next)) {
            if (findBentPath(next, steps - 1, reserved, visited, outPath)) {
                outPath.prepend(pos);
                visited.remove(pos);
                return true;
            }
        }
    }

    visited.remove(pos);
    return false;
}

// ===================== 颜色 =====================

QColor LinkGameWidget::getColor(int num) const
{
    if (num >= 1 && num <= 10) return colorsTable[num - 1];
    return Qt::gray;
}

QColor LinkGameWidget::getLightColor(int num) const
{
    if (num >= 1 && num <= 10) return lightColorsTable[num - 1];
    return QColor(220, 220, 220);
}

// ===================== 辅助函数 =====================

QPoint LinkGameWidget::cellAtPos(const QPoint &pos) const
{
    if (gridSize <= 0) return QPoint(-1, -1);
    int offsetX = (width() - gridSize * cellSize) / 2;
    int offsetY = (height() - gridSize * cellSize) / 2;
    int c = (pos.x() - offsetX) / cellSize;
    int r = (pos.y() - offsetY) / cellSize;
    if (r >= 0 && r < gridSize && c >= 0 && c < gridSize)
        return QPoint(c, r);
    return QPoint(-1, -1);
}

bool LinkGameWidget::isAdjacent(const QPoint &a, const QPoint &b) const
{
    return (qAbs(a.x() - b.x()) + qAbs(a.y() - b.y())) == 1;
}

bool LinkGameWidget::checkWin()
{
    for (const PairInfo &pair : m_pairs)
        if (!pair.connected) return false;
    return true;
}

int LinkGameWidget::getPairIndex(const QPoint &cell) const
{
    for (int i = 0; i < m_pairs.size(); i++) {
        const PairInfo &pair = m_pairs[i];
        if (!pair.connected && (pair.p1 == cell || pair.p2 == cell))
            return i;
    }
    return -1;
}

// ===================== 鼠标事件 =====================

void LinkGameWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint cell = cellAtPos(event->pos());
    if (cell.x() < 0) return;

    if (event->button() == Qt::LeftButton)
        handleLeftClick(cell);
    else if (event->button() == Qt::RightButton)
        handleRightClick();
}

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

void LinkGameWidget::handleRightClick()
{
    if (!m_isBuilding) return;

    if (m_buildPath.size() <= 1) {
        m_isBuilding = false;
        m_buildPath.clear();
    } else {
        m_buildPath.removeLast();
    }
    update();
}

// ===================== 连接操作 =====================

void LinkGameWidget::connectPair(int idx)
{
    if (idx < 0 || idx >= m_pairs.size()) return;

    PairInfo &pair = m_pairs[idx];

    // 把路径上的所有格子标记为已连接（负数）
    for (const QPoint &p : pair.path) {
        grid[p.y()][p.x()] = -pair.number;
        // 路径中间格子的颜色也设为和 pair 一样的颜色索引
        colors[p.y()][p.x()] = idx;
    }

    pair.connected = true;
    m_connectionHistory.append(idx);

    m_isBuilding = false;
    m_buildPath.clear();

    matchedPairs++;

    update();

    if (checkWin())
        emit gameWon();
}

// ===================== 绘图 =====================

void LinkGameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 背景
    if (!m_bgPixmap.isNull()) {
        painter.drawPixmap(rect(), m_bgPixmap);
    } else {
        painter.fillRect(rect(), QColor(236, 240, 241));
    }

    if (gridSize <= 0) return;

    int offsetX = (width() - gridSize * cellSize) / 2;
    int offsetY = (height() - gridSize * cellSize) / 2;

    auto cellRect = [&](int r, int c) {
        return QRect(offsetX + c * cellSize + 1,
                     offsetY + r * cellSize + 1,
                     cellSize - 2, cellSize - 2);
    };

    // 绘制每个格子
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            QRect rct = cellRect(i, j);
            int v = grid[i][j];
            int cidx = colors[i][j];

            if (v == 0) {
                // 空格（透明底 + 白色边框）
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(Qt::white, 1));
                painter.drawRect(rct);
            } else if (v > 0) {
                // 未连接的数字
                int n = v;
                QColor baseColor = (cidx >= 0) ?
                    QColor::fromHsv((cidx * 360 / qMax(1, (int)m_pairs.size())) % 360, 200, 255) :
                    getColor(n);

                painter.setPen(QPen(baseColor.darker(110), 2));
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(rct.adjusted(1, 1, -1, -1), 4, 4);

                QFont f;
                f.setPointSize(qMax(8, qMin(cellSize, cellSize) / 2));
                f.setBold(true);
                painter.setFont(f);
                painter.setPen(baseColor);
                painter.drawText(rct, Qt::AlignCenter, QString::number(n));
            } else {
                // 已连接的数字（路径格子）
                int n = -v;
                QColor baseColor = (cidx >= 0 && cidx < m_pairs.size()) ?
                    QColor::fromHsv((cidx * 360 / qMax(1, (int)m_pairs.size())) % 360, 200, 255) :
                    getColor(n);

                painter.setPen(Qt::NoPen);
                painter.fillRect(rct, baseColor);
                QColor darker = baseColor.darker(130);
                painter.setPen(QPen(darker, 1));
                painter.drawRect(rct);

                // 已连接区域，判断是否为端点
                bool isEndpoint = false;
                for (const PairInfo &pair : m_pairs) {
                    if (pair.connected) {
                        if ((pair.p1.x() == j && pair.p1.y() == i) ||
                            (pair.p2.x() == j && pair.p2.y() == i)) {
                            isEndpoint = true;
                            break;
                        }
                    }
                }

                // 只对端点的两个数字格画数字，中间路径格纯色
                if (isEndpoint) {
                    QFont f;
                    f.setPointSize(qMax(8, qMin(cellSize, cellSize) / 2));
                    f.setBold(true);
                    painter.setFont(f);
                    painter.setPen(Qt::white);
                    painter.drawText(rct, Qt::AlignCenter, QString::number(n));
                }
            }
        }
    }

    // 绘制建造路径
    if (m_isBuilding && !m_buildPath.isEmpty()) {
        int n = qAbs(grid[m_buildStart.y()][m_buildStart.x()]);
        if (n == 0) n = 1;

        int cidx = colors[m_buildStart.y()][m_buildStart.x()];
        QColor pathColor = (cidx >= 0) ?
            QColor::fromHsv((cidx * 360 / qMax(1, (int)m_pairs.size())) % 360, 255, 255) :
            getColor(n);

        // 路径格子半透明填充
        QColor fill = pathColor;
        fill.setAlpha(80);
        for (int k = 1; k < m_buildPath.size(); k++) {
            QPoint p = m_buildPath[k];
            painter.fillRect(cellRect(p.y(), p.x()).adjusted(3, 3, -3, -3), fill);
        }

        // 路径连线（虚线）
        painter.setPen(QPen(pathColor, 3, Qt::DashLine));
        for (int k = 1; k < m_buildPath.size(); k++) {
            QPointF a(offsetX + m_buildPath[k-1].x() * cellSize + cellSize / 2.0,
                      offsetY + m_buildPath[k-1].y() * cellSize + cellSize / 2.0);
            QPointF b(offsetX + m_buildPath[k].x() * cellSize + cellSize / 2.0,
                      offsetY + m_buildPath[k].y() * cellSize + cellSize / 2.0);
            painter.drawLine(a, b);
        }

        // 起点黄色高亮边框
        painter.setPen(QPen(QColor(241, 196, 15), 3));
        painter.drawRoundedRect(
            cellRect(m_buildStart.y(), m_buildStart.x()).adjusted(2, 2, -2, -2), 4, 4);
    }
}
