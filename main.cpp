#include "mainwindow.h"//包含开始游戏界面文件声明（可以调用其中的数据）
#include <QApplication>
/* windows和c++语言之间的“翻译官”  */
#include <QResource>
/*  Qt资源管理类
    负责加载.rcc格式的外部二进制资源包，
    把图片、音频……文件在运行时注册到程序中。*/
#include "config.h"//包含全局配置文件（里面有资源路径）

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //加载外部资源包
    QResource::registerResource(GAME_RES_PATH);//注册外部的.rcc二进制资源文件
    //主窗口
    MainWindow w;//创建主窗口
    w.show();//显示主窗口
    return a.exec();//启动Qt的事件循环（“翻译官”进入全天待命状态）
    /*
        exec()：阻塞调用（会一直运行直到用户关闭窗口，或调用quit())
        作用：（在运行期间）
        1.监听鼠标/键盘
        2.驱动QTimer定时器
        3.处理信号槽调用
        4.重绘界面
    */

}
