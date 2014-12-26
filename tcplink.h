#ifndef TCPLINK_H
#define TCPLINK_H
// TCPLink TCP 套接字通信类 - 实现底层与服务器和P2P的TCP通信
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include "classConstant.h"

class TCPLink : public QObject
{
    Q_OBJECT
public:
    explicit TCPLink(const ServerNode &node, QObject *parent = 0);
    ~TCPLink();

    void initasServer(void);    // 作为服务器初始化

    bool isConnected;       // 与服务器是否已连接上

    ServerNode serverNode;  // 服务器节点
    qint32 requestKind;     // 请求类型
    qint32 replyKind;       // 应答类型

//    QString replyStr;       // 应答字符串

    UserInfo userInfo;      // 用户信息
    LoginInfo loginInfo;    // 登录信息
    FriendInfo friendInfo;  //好友信息，待加为好友
    Message message;        // 会话消息
    QVector<Message> messageVect;   // 消息向量
    QVector<FriendInfo> friendVect; // 好友列表，已添加为好友

    QTcpServer *tcpServer;      // 每个用户成功登陆后建立一个服务器（以后四位数字为端口号）

    QTcpSocket *tcpClient;

    QTimer *timer;      // 计时器
    qint16 blockSize;

    void setHostAddr(const ServerNode &node)        // 设置服务器IP地址和端口号
    {
        serverNode.hostAddr = node.hostAddr;
        serverNode.hostPort = node.hostPort;
    }

    // 用户请求 - 与服务器
    void loginRequest(LoginInfo &login/* = loginInfo*/);      // 登录请求
    void queryRequest(FriendInfo &frd/* = friendInfo*/);        // 查找好友请求
    void logoutRequest(UserInfo &user/* = userInfo*/);       // 登出请求
    void messageRequest(Message &msg/* = message*/);        // 会话消息请求
    // 用户请求 - 与好友（服务器）
    void addFriendRequest(void);        // 添加好友请求

private:
    void newConnect();          // 建立新的TCP连接

    void newTCPConnection();    // 与好友（服务器）建立新连接
signals:
    void newReplySignal(qint32 replyKind);  // 新建请求信号
    void connectionFailedSignal();          // 连接失败信号
    void disconnectedSignal();              // 断开连接信号

public slots:
    void sendRequest();         // 向服务器发送信息
    void readResult();          // 从服务器读取结果
//    void serverDisconnected();  // 与服务器断开连接
//    void displayError(QAbstractSocket::SocketError socketError);        // 提示TCP socket 错误信息

    // tcpServer
//    void connectedStatus(void);         // 连接建立后状态
//    void connectionFailed(void);        // 与服务器连接失败
    void serverDisconnected(void);      // 服务器离线

//    void addFriendRequest(void);        // 添加好友请求
    void newListen(void);               // 建立 TCP 监听事件
    void acceptConnection(void);        // 接受客户端连接
    // this->FriendInfo.tcpSocket
    void displayError(QAbstractSocket::SocketError);    // 显示错误信息
    void sendData();                 // 向服务器发送请求
    void recieveData(void);             // 接受来自服务器端的数据


};

#endif // TCPLINK_H
