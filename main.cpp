//#include "qappwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <stdio.h>
#include "login.h"
#include "chatwindow.h"
#include "tcplink.h"

TCPLink *tcplink;       // tcplink 全局变量
QSqlDatabase db;        // 数据库全局变量

int main(int argc, char *argv[])
{
    QTextCodec *xcodec = QTextCodec::codecForLocale() ;
    QString exeDir = xcodec->toUnicode( QByteArray(argv[0]) ) ;
    QString BKE_CURRENT_DIR = QFileInfo( exeDir ).path() ;
    //qt has a bug in 5.2.1(windows)? so I use setLibraryPaths
    QApplication::setLibraryPaths( QApplication::libraryPaths() << BKE_CURRENT_DIR) ;
    QApplication::setStyle("cleanlooks");
    QApplication a(argc, argv);
//    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    db = QSqlDatabase::addDatabase("QSQLITE"); // 添加数据库驱动 QSQLITE SQLite
    db.setDatabaseName("database/demo.db");  // 数据库连接命名
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

//    /// access 数据库连接 ODBC .accdb/ .mdb（.accdb 暂时无法连通）成功连接之后都是SQL语句进行数据库的操作
//    QString sDb = "Database/user.mdb";//数据库文件
//    QSqlDatabase accdb = QSqlDatabase::addDatabase("QODBC");//设置数据库驱动
//    QString dsn = QString("DRIVER={Microsoft Access Driver (*.mdb)}; FIL={MS Access};DBQ=Database/user.mdb;")/*.arg(sDb)*/;//连接字符串
//    accdb.setDatabaseName(dsn);//设置连接字符串
////    accdb.setUserName("");//设置登陆数据库的用户名
////    accdb.setPassword("liuyang12");//设置密码
//    if(!accdb.open())  // 检查数据库是否打开成功
//    {
//        QMessageBox::critical(NULL, "数据库连接错误", "数据库连接失败\n请检查数据库位置");
//        return false;
//    }
//    QSqlQuery query_acc;    // 以下执行用于数据库操作的SQL语句
//    query_acc.exec("select * from login");      // 指定查找数据库中的哪张表 login 表
//    while (query_acc.next())        // 遍历输出整张表
//    {
//        int ID = query_acc.value(0).toInt();        // 自动编号
//        QString account = query_acc.value(1).toString();    // 账号
//        QString password = query_acc.value(2).toString();   // 密码
//        QString nickname = query_acc.value(3).toString();   // 昵称
//        QString avatar = query_acc.value(4).toString();     // 头像路径（相对路径）

//        qDebug() << ID << account << password << nickname << avatar;  // 全部输出
//    }
//    QWindow w;

    login log_in;
    log_in.setWindowFlags(Qt::FramelessWindowHint);     // 设置窗口无边框
    log_in.show();
//    db.close();

    return a.exec();
}
