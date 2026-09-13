//头文件定义
#ifndef BACKGROUNDDIALOG_H
#define BACKGROUNDDIALOG_H

//Qt弹窗基类
#include <QDialog>

//ui类
namespace Ui {
class BackgroundDialog;
}

class BackgroundDialog : public QDialog
{
    //信号与槽
    Q_OBJECT

public:
    BackgroundDialog(QWidget *parent = 0);
    ~BackgroundDialog();

private slots:
    //按钮信号—槽
    void onBackClicked();

private:
    //ui对象指针
    Ui::BackgroundDialog *ui;
};

#endif // BACKGROUNDDIALOG_H
