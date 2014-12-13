#ifndef TCPLINK_H
#define TCPLINK_H
// TCPLink TCP 套接字通信类 - 实现底层与服务器和P2P的TCP通信
#include <QObject>
#include <QTcpSocket>
#include "classConstant.h"

class TCPLink : public QObject
{
    Q_OBJECT
public:
    explicit TCPLink(const ServerNode &node, QObject *parent = 0);
    ~TCPLink();

    bool isConnected;       // 与服务器是否已连接上

    ServerNode serverNode;  // 服务器节点
    qint32 requestKind;     // 请求类型
    qint32 replyKind;       // 应答类型

    UserInfo userInfo;      // 用户信息
    LoginInfo loginInfo;    // 登录信息
    FriendInfo friendInfo;  //好友信息
    Message message;        // 会话消息
    QVector<Message> messageVect;   // 消息向量
    QVector<FriendInfo> friendVect; // 好友列表

    QTcpSocket *tcpClient;
    qint16 blockSize;

    void setHostAddr(const ServerNode &node)        // 设置服务器IP地址和端口号
    {
        serverNode.hostAddr = node.hostAddr;
        serverNode.hostPort = node.hostPort;
    }

    // 用户请求
    void loginRequest(LoginInfo &login/* = loginInfo*/);      // 登录请求
    void queryRequest(FriendInfo &frd/* = friendInfo*/);        // 查找好友请求
    void logoutRequest(UserInfo &user/* = userInfo*/);       // 登出请求
    void messageRequest(Message &msg/* = message*/);        // 会话消息请求

private:
    void newConnect();          // 建立新的TCP连接
signals:
    void newReplySignal(qint32 replyKind);  // 新建请求信号
    void connectionFailedSignal();          // 连接失败信号
    void disconnectedSignal();              // 断开连接信号

public slots:
    void sendRequest();         // 向服务器发送信息
    void readResult();          // 从服务器读取结果
    void serverDisconnected();  // 与服务器断开连接
    void displayError(QAbstractSocket::SocketError socketError);        // 提示TCP socket 错误信息

};

#endif // TCPLINK_H
