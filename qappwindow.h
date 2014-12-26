#ifndef QAPPWINDOW_H
#define QAPPWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include "tcplink.h"

namespace Ui {
class QAppWindow;
}

class QAppWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QAppWindow(TCPLink* tcplink, QWidget *parent = 0);
    ~QAppWindow();

    void newTCPConnect(void);           // 建立与服务器之间的连接

private slots:
    void newReply(qint32 replyKind);    // 收到新的回复信息
    void initStatus(void);              // 初始化状态
//    void connectedStatus(void);         // 连接建立后状态
//    void connectionFailed(void);        // 与服务器连接失败
    void serverDisconnected(void);      // 服务器离线

//    void addFriendRequest(void);        // 添加好友请求
    void newListen(void);               // 建立 TCP 监听事件
    void acceptConnection(void);        // 接受客户端连接
    void displayError(QAbstractSocket::SocketError);    // 显示错误信息
    void sendRequest();                 // 向服务器发送请求
    void recieveData(void);             // 接受来自服务器端的数据

    void on_Button_queryFriend_clicked();

    void on_action_Logout_triggered();

private:
    Ui::QAppWindow *ui;

    QTcpServer *tcpServer;      // TCP Server
//    QTcpSocket *tcpSocket;      // TCP Socket
    QTimer *timer;              // 计时器
//    QTcpSocket *tcpSender;      // TCP Sender

    TCPLink *tcplink;

signals:
    void reLoginSignal(void);     // 重新登录请求
    void disconnectedSignal(void);
};

#endif // QAPPWINDOW_H
