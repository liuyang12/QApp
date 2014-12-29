//#include "qappwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <stdio.h>
#include "login.h"
#include "chatwindow.h"
#include "tcplink.h"

TCPLink *tcplink;       // tcplink 全局变量

int main(int argc, char *argv[])
{
    QApplication::setStyle("cleanlooks");
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    // 查看Qt已经安装的所有数据库驱动
    qDebug() << "Available drivers: ";
    QStringList drivers = QSqlDatabase::drivers();
    foreach (QString driver, drivers) {
        qDebug() << "\t" << driver;
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"); // 添加数据库驱动 QSQLITE SQLite
    db.setDatabaseName("Database/demo.db");  // 数据库连接命名
    if(!db.open())  // 检查数据库是否打开成功
    {
        QMessageBox::critical(NULL, "数据库连接错误", "数据库连接失败\n请检查数据库位置");
        return false;
    }
    QSqlQuery query;    // 以下执行用于数据库操作的SQL语句
    query.exec("select * from login");      // 指定查找数据库中的哪张表 login 表
    while (query.next())        // 遍历输出整张表
    {
        int ID = query.value(0).toInt();        // 自动编号
        QString account = query.value(1).toString();    // 账号
        QString password = query.value(2).toString();   // 密码
        QString nickname = query.value(3).toString();   // 昵称
        QString avatar = query.value(4).toString();     // 头像路径（相对路径）

        qDebug() << ID << account << password << nickname << avatar;  // 全部输出
    }

//    QWindow w;
    login log_in;
//    log_in.setWindowOpacity(1);
    log_in.setWindowFlags(Qt::FramelessWindowHint);     // 设置窗口无边框
//    log_in.setAttribute(Qt::WA_TranslucentBackground);  // 设置窗口透明，支持 .png 格式图片，在styleSheet中修改
//    log_in.setStyleSheet("");
    log_in.show();
    db.close();

    return a.exec();
}
