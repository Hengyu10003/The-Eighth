#ifndef MAZEWIDGET_H
#define MAZEWIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QPixmap>
#include <QVector>
#include <QPair>

#include "config.h"

class MazeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MazeWidget(QWidget *parent = nullptr, int level = 1, QSize size = QSize(800, 600));
    ~MazeWidget() override;

signals:
    void mazeCompleted();
    void returnToMenu();
    void gamePaused(bool paused);

private slots:
    void animatePlayer();
    void onAutoPath();
    void autoStep();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void generateMaze();
    bool isValidMove(int x, int y);

    int mazeWidth;
    int mazeHeight;
    int cellSize;
    int level;
    int playerX;
    int playerY;
    int exitX;
    int exitY;
    bool **maze;
    QTimer *animationTimer;
    QPixmap m_playerPixmapR[4];  // 向右动画
    QPixmap m_playerPixmapL[4];  // 向左动画
    QPixmap m_doorPixmap;
    QPixmap m_wallPixmap;
    QPixmap m_bgPixmap;
    int m_playerFrame;
    int m_lastDirectionX;  // 1=右, -1=左
    QTimer *m_autoPathTimer;
    QVector<QPair<int,int>> m_autoPath;
    int m_autoPathIndex;
    bool m_paused;
    bool m_autoPathWasRunning;  // 记录自动寻路是否在运行
};

#endif // MAZEWIDGET_H
