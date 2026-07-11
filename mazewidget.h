#ifndef MAZEWIDGET_H
#define MAZEWIDGET_H

#include <QWidget>
#include <QKeyEvent>
#include <QTimer>

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
};

#endif // MAZEWIDGET_H
