#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include "classConstant.h"
#include "tcplink.h"

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = 0);
    ~login();
    bool isConnected;           // status whether client is connected with server
    ServerNode serverNode;      // 服务器节点
    LoginInfo loginInfo;        // 登录信息
    TCPLink *tcplink;           // TCP Socket通信类
//    QHostAddress hostAddr;      // 服务器地址
//    quint16 hostPort;               // 服务器端口号
//    QTcpSocket *tcpSocket;       // UdpSocket
//    QString LogNumber;         // 登陆用户名
//    QString LogPassword;       // 登陆密码
//    QString LogInfo;
//    QString Reply;              // 收到的服务器端的回复

    QPoint dragPosition;

private slots:
//    void sendRequest(void);     // 发送请求，TCP初始化连接
//    void readResult(void);      // 读取服务器端的结果
//    void ServerDisconnected(void);  // 服务器端断开连接
//    void displayError(QAbstractSocket::SocketError);    // 连接错误
////    void processPendingDatagrams(void);     // 处理接受到信息
////    void on_buttonConfirm_accepted();
    void newReply(qint32 replyKind);    // 收到新的回复信息
    void initStatus(void);              // 初始化状态
//    void connectedStatus(void);         // 连接建立后状态
//    void connectionFailed(void);        // 与服务器连接失败
    void serverDisconnected(void);      // 服务器离线

    void reLogin(void);                 // 用户重新登录

    void on_EditNumber_textChanged(const QString &arg1);

    void on_EditPassword_textChanged(const QString &arg1);

    void on_buttonConfirm_clicked();

    //void on_pushButton_clicked();

    void on_Button_ConfigServer_clicked();

//拖动窗口

    void mousePressEvent(QMouseEvent *event);

    void  mouseMoveEvent(QMouseEvent *event);


    //void on_close_clicked();

    //void on_giflable_linkActivated(const QString &link);

    //    void init();

    //    bool eventFilter(QObject*target, QEvent*e);

    void on_close_button_clicked();

private:
    Ui::login *ui;
//    qint16 blockSize;
//    void newConnect(void);      // TCP初始化连接
signals:
    void loggedinSignal(void);      // 用户已经登录信号
    void disconnectedSignal(void);  // 服务器未连接
//    void connectionFailedSignal(void);
};

#endif // LOGIN_H
