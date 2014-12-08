#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include <QHostAddress>
#include <QTcpSocket>

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
    QHostAddress hostAddr;      // 服务器地址
    quint16 hostPort;               // 服务器端口号
    QTcpSocket *tcpSocket;       // UdpSocket
    QString LogNumber;         // 登陆用户名
    QString LogPassword;       // 登陆密码
    QString LogInfo;
    QString Reply;              // 收到的服务器端的回复

private slots:
    void sendRequest(void);     // 发送请求，TCP初始化连接
    void readResult(void);      // 读取服务器端的结果
    void ServerDisconnected(void);  // 服务器端断开连接
    void displayError(QAbstractSocket::SocketError);    // 连接错误
//    void processPendingDatagrams(void);     // 处理接受到信息
    void on_buttonConfirm_accepted();

    void on_EditNumber_textChanged(const QString &arg1);

    void on_EditPassword_textChanged(const QString &arg1);

private:
    Ui::login *ui;
    qint16 blockSize;
    void newConnect(void);      // TCP初始化连接
signals:
    void disconnectedSignal(void);  // 服务器未连接
    void connectionFailedSignal(void);
};

#endif // LOGIN_H
