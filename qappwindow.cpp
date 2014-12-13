#include "qappwindow.h"
#include "ui_qappwindow.h"

QAppWindow::QAppWindow(TCPLink* tcplink, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QAppWindow),
    tcplink(tcplink)
{
    ui->setupUi(this);
    ui->Edit_FriendAccount->setText("2012011");
    tcplink->userInfo.account = tcplink->loginInfo.account;
    tcplink->userInfo.status = tcplink->loginInfo.status = ONLINE;

    QAppWindow::initStatus();
    connect(ui->Edit_FriendAccount, SIGNAL(returnPressed()), this, SLOT(on_Button_queryFriend_clicked()));
    // 接收TCP Socket 通信返回信息信号与 this 的槽相连
    connect(tcplink, SIGNAL(newReplySignal(qint32)), this, SLOT(newReply(qint32)));
    connect(tcplink, SIGNAL(connectionFailedSignal()), this, SLOT(initStatus()));
    connect(tcplink, SIGNAL(disconnectedSignal()), this, SLOT(serverDisconnected()));

}

QAppWindow::~QAppWindow()
{
    delete ui;
}
// 主界面初始状态设置
void QAppWindow::initStatus()
{
    // 主界面初始状态设置


}
// 与服务器断开连接
void QAppWindow::serverDisconnected()
{
    // 处理与服务器断开连接的相关问题
    emit disconnectedSignal();
}
// 收到新回复 - 服务器或者好友服务器
void QAppWindow::newReply(qint32 replyKind)
{

    switch (replyKind) {
    /** 查找好友结果 */
    case FRIEND_ONLINE:
        qDebug() << "已找到用户" << tcplink->friendInfo.account << "IP地址: " << tcplink->friendInfo.node.hostAddr;
        QMessageBox::information(this, tr("info"), tr("用户在线"), QMessageBox::Ok);
        break;
    case FRIEND_OFFLINE:
        qDebug() << "用户" << tcplink->friendInfo.account << "离线";
        QMessageBox::information(this, tr("info"), tr("用户在线"), QMessageBox::Ok);
        break;
    case FRIEND_NO_ACCOUNT:
        qDebug() << "用户" << tcplink->friendInfo.account << "查找结果：查找账户错误";
        QMessageBox::information(this, tr("info"), tr("查找账户错误"), QMessageBox::Ok);
        break;
    case NO_REPLY:
        qDebug() << "用户" << tcplink->friendInfo.account << "查找结果：未收到服务器回复";
        QMessageBox::information(this, tr("info"), tr("未收到回复"), QMessageBox::Ok);
        break;
    /** 登出结果 */
    case LOGOUT_SUCCESS:
        qDebug() << "用户" << tcplink->loginInfo.account << "下线成功";
        QMessageBox::information(this, tr("info"), tr("下线成功"), QMessageBox::Ok);
        // 成功下线，关闭主界面，重新登录
        emit reLoginSignal();
        this->close();
        break;
    case LOGOUT_FAILED:
        qDebug() << "用户" << tcplink->loginInfo.account << "离线状态";
        QMessageBox::information(this, tr("info"), tr("离线状态"), QMessageBox::Ok);
        break;
    /** 好友请求 */

    default:
        break;
    }
}

// 查找好友请求
void QAppWindow::on_Button_queryFriend_clicked()
{
    if(ui->Edit_FriendAccount->text().isEmpty() || ui->Edit_FriendAccount->text() == "2012011")
    {
        QMessageBox::critical(this, tr("ERROR"), tr("好友账号不能为空"), QMessageBox::Ok);
        return ;
    }
    tcplink->friendInfo.account = ui->Edit_FriendAccount->text();
//    tcplink->requestKind = QUERY;
    tcplink->queryRequest(tcplink->friendInfo);
}
// 登出请求
void QAppWindow::on_action_Logout_triggered()
{
    tcplink->userInfo.status = tcplink->loginInfo.status = OFFLINE;
    tcplink->userInfo.account = tcplink->loginInfo.account;
//    tcplink->requestKind = LOGOUT;
    tcplink->logoutRequest(tcplink->userInfo);
}
