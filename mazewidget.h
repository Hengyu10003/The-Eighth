#ifndef MAZEWIDGET_H
#define MAZEWIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QPixmap>

#include "config.h"

class MazeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MazeWidget(QWidget *parent = nullptr, int level = 1, QSize size = QSize(800, 600));
    ~MazeWidget() override;

    void setLevel(int level);
    void reset();
signals:
    void mazeCompleted();
    void returnToMenu();

private slots:
    void animatePlayer();

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
    QPixmap m_playerPixmap[4];
    QPixmap m_doorPixmap;
    QPixmap m_wallPixmap;
    QPixmap m_bgPixmap;
    int m_playerFrame;
};

#endif // MAZEWIDGET_H
