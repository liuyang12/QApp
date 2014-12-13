//#include "qappwindow.h"
#include <QApplication>
#include <stdio.h>
#include "login.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QWindow w;
    login log_in;
    log_in.show();

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
