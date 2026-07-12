#ifndef LINKGAMEWIDGET_H
#define LINKGAMEWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QColor>
#include <QPoint>
#include <QVector>
#include <QSet>
#include <QPixmap>

#include "config.h"

class LinkGameWidget : public QWidget
{
    Q_OBJECT

public:
    LinkGameWidget(QWidget *parent = 0, int difficulty = 1, QSize size = QSize(800, 600));
    ~LinkGameWidget();

    void setDifficulty(int difficulty);
    void reset();

signals:
    void gameWon();
    void gameLost();
    void returnToMenu();
    void gamePaused(bool paused);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    struct PairInfo {
        int number;
        QPoint p1, p2;
        QVector<QPoint> path;
        bool connected;
    };

    void initGame();
    bool checkWin();
    void handleLeftClick(const QPoint &cell);
    void handleRightClick();
    void connectPair(int idx);
    int getPairIndex(const QPoint &cell) const;
    bool isAdjacent(const QPoint &a, const QPoint &b) const;
    QPoint cellAtPos(const QPoint &pos) const;

    // Board generation
    bool generateBoard();
    bool findBentPath(const QPoint &pos, int steps,
                      const QSet<QPoint> &reserved,
                      QSet<QPoint> &visited,
                      QVector<QPoint> &outPath) const;

    QColor getColor(int num) const;
    QColor getLightColor(int num) const;

    int gridSize;
    int numRange;
    int cellSize;
    int difficulty;
    int **grid;
    int **colors;        // 存储每个格子的颜色索引（pair index）
    int totalPairs;
    int matchedPairs;

    // 建造模式
    bool m_isBuilding;
    QPoint m_buildStart;
    QVector<QPoint> m_buildPath;

    // 成对管理
    QVector<PairInfo> m_pairs;
    QVector<int> m_connectionHistory;
    QPixmap m_bgPixmap;
    bool m_paused;
};

#endif // LINKGAMEWIDGET_H
