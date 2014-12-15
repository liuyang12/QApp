//#include "qappwindow.h"
#include <QApplication>
#include <stdio.h>
#include "login.h"
#include "chatwindow.h"

int main(int argc, char *argv[])
{
    QApplication::setStyle("cleanlooks");
    QApplication a(argc, argv);
//    QWindow w;
    login log_in;
//    log_in.setWindowOpacity(1);
    log_in.setWindowFlags(Qt::FramelessWindowHint);     // 设置窗口无边框
//    log_in.setAttribute(Qt::WA_TranslucentBackground);  // 设置窗口透明，支持 .png 格式图片，在styleSheet中修改
//    log_in.setStyleSheet("");
    log_in.show();
    chatWindow chat;
//    chat.setWindowOpacity(1);
    chat.setWindowFlags(Qt::FramelessWindowHint);       // 设置窗口无边框，暂时可通过alt + f4关闭窗口，之后增加关闭 label
//    chat.setAttribute(Qt::WA_TranslucentBackground);    // 设置窗口透明，支持 .png 格式图片，在styleSheet中修改
    chat.show();

    return a.exec();

//    login log_in;   // 登陆界面 todo: 1.美化；2.多人登陆
//    // 登陆成功进入主界面
//    if(log_in.exec() == QDialog::Accepted)
//    {
//        w.show();
//        return a.exec();
//    }
//    else
//        return 0;
}
