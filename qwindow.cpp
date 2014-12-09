#include "qwindow.h"
#include "ui_qwindow.h"

#include <QTcpSocket>
#include <QMessageBox>

QWindow::QWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QWindow)
{
    ui->setupUi(this);
    ui->Edit_FriendNumber->setText("2012011");
    // 响应回车键
    connect(ui->Edit_FriendNumber, SIGNAL(returnPressed()), this, SLOT(on_button_SearchFriend_clicked()));
//    QMessageBox::warning(0, tr("info"), tr("Hello World!"), QMessageBox::Ok);

}

QWindow::~QWindow()
{
    delete ui;
}

void QWindow::on_button_SearchFriend_clicked()
{
    /// TODO TCP socket 通信应该封装成一个类，作为通信底层
    // 查找好友 "q2012011xxx"
    // 好友在线返回 ip 地址，没有找到或者不在线返回 'n'
    QMessageBox::information(this, tr("info"), tr("查找好友"), QMessageBox::Yes);

}

void QWindow::on_action_Logout_triggered()
{
    /// TODO TCP socket 通信应该封装成一个类，作为通信底层
    // 登出 "logout2012011xxx"
    //  下线成功返回 "loo"，不成功说明与服务器通信中断，服务器端是否应该判断在线用户当前时刻是否在线并且实时更新
    QMessageBox::information(this, tr("info"), tr("用户登出"), QMessageBox::Yes);

}
