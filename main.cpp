#include "mainwindow.h"
#include <QApplication>
#include <QResource>
#include "config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 先加载外部资源包
    QResource::registerResource(GAME_RES_PATH);
    // 再创建窗口
    MainWindow w;
    w.show();
    return a.exec();
}
