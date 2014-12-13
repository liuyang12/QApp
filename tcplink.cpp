#include "tcplink.h"

TCPLink::TCPLink(const ServerNode &node, QObject *parent/* = 0*/) :
    QObject(parent), serverNode(node)
{
    isConnected = false;
    tcpClient = new QTcpSocket(this);

    // 信号与槽连接
    connect(tcpClient, SIGNAL(connected()), this, SLOT(sendRequest()));
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(readResult()));
    connect(tcpClient, SIGNAL(disconnected()), this, SLOT(serverDisconnected()));
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

}

TCPLink::~TCPLink()
{

}
// 新建 TCP 连接，连接到服务器
void TCPLink::newConnect()
{
    blockSize = 0;
    if(!isConnected)
    {
        tcpClient->abort();
        tcpClient->connectToHost(serverNode.hostAddr, serverNode.hostPort);
    }
    else
        TCPLink::sendRequest();
}
// 发送请求 tcpClient readyRead() 信号之后 - 根据不同的发送请求
void TCPLink::sendRequest()
{
    // 对不同的请求信息进行相应的处理
    QString LoginInfo;      // 登录请求，请求信息
    QString QueryInfo;      // 查找请求，请求信息
    QString LogoutInfo;     // 登出请求，请求信息
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    switch (requestKind) {
    case LOGIN:     // 登录请求
        // 登录请求 "$account_$password"
        LoginInfo = loginInfo.account + "_" + loginInfo.password;
        qDebug() << LoginInfo;
        tcpClient->write(LoginInfo.toStdString().c_str());      // 发送登录请求
        break;
    case QUERY:     // 查找好友请求
        // 查找好友请求 "q$account"
        QueryInfo = "q" + friendInfo.account;
        qDebug() << QueryInfo;
        tcpClient->write(QueryInfo.toStdString().c_str());      // 发送查找好友请求
        break;
    case LOGOUT:    // 登出请求
        // 登出请求 "logout$account"
        LogoutInfo = "logout" + loginInfo.account;
        qDebug() << LogoutInfo;
        tcpClient->write(LogoutInfo.toStdString().c_str());     // 发送登出请求
        break;
    case MESSAGE:   // 发送消息请求
//        QByteArray block;
//        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_3);    // Qt 版本是否关键，是否向下兼容


        out << (qint16) 0;
        out << requestKind;
        out << message;

        out.device()->seek(0);  // 重新定位到初始
        out << (qint16)(block.size() - sizeof(qint16));// 首部 发送信息长度
        tcpClient->write(block);
        qDebug() << block.data();

    default:
        qDebug() << "requestKind ERROR!";
        break;
    };
    if(!isConnected)
        isConnected = true;
}

void TCPLink::readResult()
{
    QByteArray qba;
    QString Reply;
    QDataStream in(tcpClient);
    in.setVersion(QDataStream::Qt_5_3); // Qt 版本是否关键，是否向下兼容
    switch (requestKind) {
    case LOGIN:
    case QUERY:
    case LOGOUT:
        qba = tcpClient->readAll();
        Reply = QVariant(qba).toString();
        break;
    case MESSAGE:
        if(blockSize == 0)
        {
            if(tcpClient->bytesAvailable() < (int)sizeof(qint16))
                return ;
            in >> blockSize;
        }
        if(tcpClient->bytesAvailable() < blockSize)
            return;
//        in >> replyKind;        // 处理发送信息
        in >> Reply;

        blockSize = 0;
        emit newReplySignal(replyKind); // 发布
        break;
    default:
        break;
    }
    qDebug() << "New reply: " << Reply;
    switch (requestKind) {
    case LOGIN:
        if(Reply == "lol")
        {
            replyKind = LOGIN_SUCCESS;
        }
        else if(Reply == "Incorrect No.")
        {
            replyKind = LOGIN_NO_ACCOUNT;
        }
        else if(Reply == "Please send the correct message.")
        {
            replyKind = LOGIN_WRONG_INPUT;
        }
        else if(Reply == "")
        {
            replyKind = NO_REPLY;
            loginInfo.status = OFFLINE;
            qDebug() << "用户" << loginInfo.account << "未收到用户请求结果，请检查：\n1. 用户网络状态\n2. 服务器地址和端口号（默认166.111.180.60:8000）";
//            QMessageBox::information(this, tr("info"), tr("未收到用户请求结果，请检查：\n1. 用户网络状态\n2. 服务器地址和端口号（默认166.111.180.60:8000）"));
        }
        else
        {
            replyKind = NO_REPLY;
            loginInfo.status = OFFLINE;
            qDebug() << "用户" << loginInfo.account << "404 not found!\ndefault message";
//            QMessageBox::information(this, tr("info"), tr("404 not found!\ndefault message"));

        }
        break;
    case QUERY:
        // 正则表达式判断返回是否为IP地址
        if(ipRegExp.exactMatch(Reply))
        {
            replyKind = FRIEND_ONLINE;
            friendInfo.status = ONLINE;
            friendInfo.node.hostAddr = Reply;
        }
        else if(Reply == "n")
        {
            replyKind = FRIEND_OFFLINE;
            friendInfo.status = OFFLINE;
        }
        else if(Reply == "")
        {
            replyKind = NO_REPLY;
            friendInfo.status = OFFLINE;
        }
        else
        {
            replyKind = FRIEND_NO_ACCOUNT;
            friendInfo.status = OFFLINE;
        }
        break;
    case LOGOUT:
        if(Reply == "loo")
        {
            replyKind = LOGOUT_SUCCESS;
            loginInfo.status = OFFLINE;
        }
        else if(Reply == "")
        {
            replyKind = NO_REPLY;
            friendInfo.status = OFFLINE;
            qDebug() << "用户" << friendInfo.account << "查找结果：未收到服务器回复";
//            QMessageBox::information(this, tr("info"), tr("未收到回复"), QMessageBox::Ok);
        }
        else
        {
            replyKind = LOGOUT_FAILED;
//            LoginInfo.status = OFFLINE;
        }
        break;
    case MESSAGE:
        // TODO: send messages
        message.text = Reply;

        break;
    default:
        break;
    }
    blockSize = 0;
    emit newReplySignal(replyKind);

}
// 显示错误提示信息
void TCPLink::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(NULL, tr("Client"),
            tr("The host was not found. Please check the "
            "host name and port settings."));
        emit connectionFailedSignal();
        break;
    case QAbstractSocket::ConnectionRefusedError:

        QMessageBox::information(NULL, tr("Client"),
            tr("The connection was refused by the peer. "
            "Make sure the fortune server is running, "
            "and check that the host name and port "
            "settings are correct."));
        emit connectionFailedSignal();
        break;
    default:
        QMessageBox::information(NULL, tr("Client"),
            tr("For unknown reasons, connected failed"));
        emit connectionFailedSignal();
    }
}
// 与服务器失去连接
void TCPLink::serverDisconnected()
{
    emit disconnectedSignal();
}
// 登录请求
void TCPLink::loginRequest(LoginInfo &login/* = loginInfo*/)
{
    requestKind = LOGIN;
    loginInfo = login;
    newConnect();
}
// 查找好友请求
void TCPLink::queryRequest(FriendInfo &frd/* = friendInfo*/)
{
    requestKind = QUERY;
    friendInfo = frd;
    newConnect();
}
// 登出请求
void TCPLink::logoutRequest(UserInfo &user/* = userInfo*/)
{
    requestKind = LOGOUT;
    userInfo = user;
    newConnect();
}
// 聊天请求
void TCPLink::messageRequest(Message &msg/* = message*/)
{
    requestKind = MESSAGE;
    message = msg;
    newConnect();
}
