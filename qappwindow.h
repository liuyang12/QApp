#ifndef QAPPWINDOW_H
#define QAPPWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
//#include <QTimer>
//#include "tcplink.h"
#include "classConstant.h"

namespace Ui {
class QAppWindow;
}

class QAppWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QAppWindow(/*TCPLink* tcplink, */QWidget *parent = 0);
    ~QAppWindow();

    void newTCPConnect(void);           // 建立与服务器之间的连接

private slots:
    void newReply(qint32 replyKind);    // 收到新的回复信息
    void initStatus(void);              // 初始化状态
////    void connectedStatus(void);         // 连接建立后状态
////    void connectionFailed(void);        // 与服务器连接失败
    void serverDisconnected(void);      // 服务器离线

    void on_Button_queryFriend_clicked();

    void on_action_Logout_triggered();

//文件传输
    void acceptConnection();  //建立连接
    void updateServerProgress();  //更新进度条，接收数据
    void displayError(QAbstractSocket::SocketError socketError);//显示错误

private:
    Ui::QAppWindow *ui;

    QTcpServer *tcpServer;      // TCP Server
    QTcpSocket *tcpClient;      // TCP Socket
//    QTimer *timer;              // 计时器
////    QTcpSocket *tcpSender;      // TCP Sender

//    TCPLink *tcplink;
    TSFile ReceiveFile;

signals:
    void reLoginSignal(void);     // 重新登录请求
    void disconnectedSignal(void);

    void FriendInfoSignal(FriendInfo);//好友信息
};

#endif // QAPPWINDOW_H
