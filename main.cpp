#include "mainwindow.h"//包含开始游戏界面文件声明（可以调用其中的数据）
#include <QApplication>/* windows和c++语言之间的“翻译官”  */
#include <QResource>

#include "config.h"//包含全局配置文件（里面有资源路径）

int main(int argc, char *argv[])
{
    //初始化Qt运行环境，管理全局资源和事件循环
    QApplication a(argc, argv);
    //加载外部资源包
    QResource::registerResource(GAME_RES_PATH);//注册外部的.rcc二进制资源文件
    //主窗口
    MainWindow w;//创建主窗口
    w.show();//显示主窗口
    return a.exec();//启动Qt的事件循环（“翻译官”进入全天待命状态）
}
