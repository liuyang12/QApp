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

    // 用户请求
    void loginRequest(const LoginInfo &login);      // 登录请求
    void queryRequest(const QString &query);        // 查找好友请求
    void logoutRequest(const UserInfo &user);       // 登出请求
    void messageRequest(const Message &msg);        // 会话消息请求

private:
    void newConnect();          // 建立新的TCP连接
signals:

public slots:

};

#endif // TCPLINK_H
