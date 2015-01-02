#include "tcplink.h"
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

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
// 作为服务器初始化
void TCPLink::initasServer()
{
    FriendInfo myself;
    myself.account = loginInfo.account;
    myself.status = loginInfo.status;
    friendVect.push_back(myself);  // myself = friendVect[0]，第 0 位好友为其本身
    userInfo.account = loginInfo.account;
    userInfo.status = loginInfo.status = ONLINE;
    timer = new QTimer;
    tcpServer = new QTcpServer(this);
//    this->tcpSender = new QTcpSocket(this);
    friendInfo.tcpSocket = new QTcpSocket(this);

    newListen();    // 监听客户端请求。端口号 为学号后四位数字
    // 当有客户端访问时，发出newConnection() 信号，acceptConnection() 处理该新建连接信号
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    // 当tcpSocket在接受客户端连接时出现错误时，displayError(QAbstractSocket::SocketError) 处理该信号
    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    // 获取发送的数据信息
    /** friendInfo.tcpSocket = tcpSender */
    connect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendData()));     // 已连接向服务器发送请求
    connect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
//    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));    // 处理错误信息


}
// 切断之前的信号槽连接，以便建立新的连接
void TCPLink::disconnectfriendSocket()
{
    disconnect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    // 获取发送的数据信息
    /** friendInfo.tcpSocket = tcpSender */
    disconnect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendData()));     // 已连接向服务器发送请求
    disconnect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
}

// 监听客户端请求，端口号 为学号后四位数字，以
void TCPLink::newListen()
{
    // 监听是否有客户端来访，且对任意来访者进行监听，端口号 为学号后四位数字
    if(!tcpServer->listen(QHostAddress::Any, /*6666*/getPortNumber(loginInfo.account)))
    {
        qDebug() << tcpServer->errorString();
//        close();
        return ;
    }
}
// 当有客户端访问时，发出newConnection() 信号，acceptConnection() 处理该新建连接信号
void TCPLink::acceptConnection()
{
//    if(friendInfo.isConnected == false)
//    {
        // 当有客户来访时，将tcpSocket接受tcpServer建立的socket
        loginInfo.request = GET_FRIEND;
        friendInfo.tcpSocket = tcpServer->nextPendingConnection();
        friendInfo.node.hostAddr = friendInfo.tcpSocket->peerAddress().toString();   // 获取客户端IP地址
        //    friendInfo.node.hostPort = friendInfo.tcpSocket->peerPort();  // 获取客户端端口号
        qDebug() << "accept connection from client: " << friendInfo.node.hostAddr;
        // 当tcpSocket在接受客户端连接时出现错误时，displayError(QAbstractSocket::SocketError) 处理该信号
        connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
        // 获取发送的数据信息
        /** friendInfo.tcpSocket = tcpSender */
        connect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendData()));     // 已连接向服务器发送请求
        connect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
        //    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));    // 处理错误信息
//        friendInfo.isConnected = true;
//    }
//    else
//    {
////        friendInfo.isConnected = true;
//    }

}
// 当tcpSocket在接受客户端连接时出现错误时，displayError(QAbstractSocket::SocketError) 处理该信号
//void TCPLink::displayError(QAbstractSocket::SocketError)
//{
//    qDebug() << friendInfo.tcpSocket->errorString()/*friendInfo.tcpSocket->errorString()*/;
//    friendInfo.tcpSocket->close();
//}
// 建立连接到该好友的
void TCPLink::newTCPConnection()
{
    if(friendInfo.isConnected == false)
    {
        friendInfo.tcpSocket->abort();
        friendInfo.tcpSocket->connectToHost(friendInfo.node.hostAddr, getPortNumber(friendInfo.account) /*friendInfo.node.hostPort*/); // 连接到 待加好友服务器，IP地址为回复，端口号为好友账号后四位
        qDebug() << "connect to host: IP " << friendInfo.node.hostAddr << " port " << getPortNumber(friendInfo.account);

//        friendInfo.isConnected = true;
    }
    else
    {
        TCPLink::sendData();
    }

}
// 发送请求
void TCPLink::sendData()
{

    switch (requestKind) {
    case ADD_FRIEND:       // 发送好友请求
        qDebug() << loginInfo.account.toStdString().c_str();
        friendInfo.tcpSocket->write(loginInfo.account.toStdString().c_str());
        break;
//    case :

//        break;
    default:
        break;
    }
}
// 读取请求
void TCPLink::recieveData()
{
   QByteArray qba;
   QString Reply;
   qba = friendInfo.tcpSocket->readAll();
   Reply = QVariant(qba).toString();
   qDebug() << Reply;
   if(Reply.size() == 10/*accountRegExp.exactMatch(Reply)*/)       // 回复匹配学号
   {
//       chatWindow *chat;
       friendInfo.account = Reply;
//       friendInfo.node = friendInfo.tcpSocket-
       replyKind = ADDFRIEND_REQUEST;

   }
   else if(Reply == "ADD")  // 好友接收好友请求
   {
//       chatWindow *chat;
       friendVect.push_back(friendInfo);
       /** 将好友信息显示在主界面窗口上 */
//       chat = new chatWindow;
//       chat->show();
       replyKind = ADDFRIEND_SUCCESS;       // 成功添加好友

   }
   else if(Reply == "CANCEL")// 好友取消好友请求
   {
       replyKind = ADDFRIEND_DENY;          // 好友拒绝添加好友请求
   }
   else
   {
       replyKind = NO_REPLY;
   }
   emit newReplySignal(replyKind);   // 发送接收到新的数据信号
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
    case TRAVELSAL: // 遍历好友在线状况
        // 查找好友请求 "q$account"
        QueryInfo = "q" + travelsalFriend.account;
        qDebug() << QueryInfo;
        tcpClient->write(QueryInfo.toStdString().c_str());      // 发送查找好友请求
        tcpClient->waitForReadyRead(1000);// 等待发送完成
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
    // TODO: 处理向服务器发送请求超时的情况，计时并在超时后发出警告
    QByteArray qba;
    QString Reply;
    QDataStream in(tcpClient);
    in.setVersion(QDataStream::Qt_5_3); // Qt 版本是否关键，是否向下兼容
    switch (requestKind) {
    case LOGIN:
    case QUERY:
    case TRAVELSAL:
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
        if(Reply == "n")
        {
            replyKind = FRIEND_OFFLINE;
            friendInfo.status = OFFLINE;
        }
        else if(ipRegExp.exactMatch(Reply))
        {
            replyKind = FRIEND_ONLINE;
            friendInfo.status = ONLINE;
            friendInfo.node.hostAddr = Reply;       // IP 地址为回复信息
            friendInfo.node.hostPort = getPortNumber(friendInfo.account);   // 端口号为 账号后4位数字
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
    case TRAVELSAL: // 遍历好友在线状态
        if(Reply == "n")    // 服务器返回 "n"，则好友离线
        {
            replyKind = FRIEND_OFFLINE;
            travelsalFriend.status = OFFLINE;
        }
        else if(ipRegExp.exactMatch(Reply)) // 匹配 IP 地址正则表达式
        {
            replyKind = FRIEND_ONLINE;
            travelsalFriend.status = ONLINE;
            travelsalFriend.node.hostAddr = Reply;      // 服务器返回 IP 地址
            travelsalFriend.node.hostPort = getPortNumber(travelsalFriend.account); // 好友作为服务器的端口号即为账号后4位数字
        }
        else if(Reply == "")
        {
            replyKind = NO_REPLY;
            travelsalFriend.status = OFFLINE;
        }
        else
        {
            replyKind = FRIEND_NO_ACCOUNT;
            travelsalFriend.status = OFFLINE;
        }
        blockSize = 0;
        emit travelsalReplySignal(replyKind);   // 遍历好友回复信号
        return ;    // 直接返回，不发出其他回复信号
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
/// 与（主）服务器通信，向服务器发送请求
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
// 遍历所有好友请求
bool TCPLink::travelsalRequest(void)
{
//    requestKind = TRAVELSAL;    // 遍历好友
//    travelsalFriend.account = account; // 置为当前遍历好友的账号
//    newConnect();
    blockSize = 0;
    bool selfincluded = false;      // 自身是否包含在数据库中
    bool allReplyed = true;     // 所有查询均收到服务器回复
    if(!isConnected)
    {
        tcpClient->abort();
        tcpClient->connectToHost(serverNode.hostAddr, serverNode.hostPort);
    }
    else
    {
        QSqlQuery query;    // 数据库查询
        query.exec("select * from friends");    // 指定查找数据库中 friends 表

        while(query.next()) // 遍历整张表
        {
            FriendInfo tempfriend;
            // 0 ID
            tempfriend.account = query.value(1).toString();     // 1 账号
            tempfriend.name = query.value(2).toString();        // 2 昵称
            tempfriend.avatar = query.value(3).toString();      // 3 头像路径，相对路径
            // 4 IP
            // 5 status
            tempfriend.block = query.value(6).toString();       // 6 所在分组
            tempfriend.group = query.value(7).toString();       // 7 所在群
//            travelsalRequest(tempfriend.account);  // 遍历好友在线状态请求
            QString QueryInfo;
            QueryInfo = "q" + tempfriend.account;
            qDebug() << QueryInfo;
            tcpClient->write(QueryInfo.toStdString().c_str());
            if(tcpClient->waitForReadyRead(500))   // 等待数据可读 500ms超时
            {   // 有数据
                QByteArray qba;
                QString Reply;
                qba = tcpClient->readAll();
                Reply = QVariant(qba).toString();
                QSqlQuery qr;
                qDebug() << Reply;
                if(Reply == "n")
                {
                    // 好友不在线
                    qr.prepare("update friends set status = :status where account = :account");
                    qr.bindValue(":account", tempfriend.account);
                    qr.bindValue(":status", OFFLINE);    // 在线状态 status
                    qr.exec();
//                    query.exec();
                    tempfriend.status = OFFLINE;
                    qDebug() << "好友" << tempfriend.account << "离线";
//                    replyKind = FRIEND_OFFLINE;
//                    friendInfo.status = OFFLINE;
                }
                else if(ipRegExp.exactMatch(Reply))
                {
                    qr.prepare("update friends set status = :status where account = :account");
                    qr.bindValue(":account", tempfriend.account);
                    qr.bindValue(":status", ONLINE);    // 在线状态 status
                    qr.exec();
                    qr.prepare("update friends set IP = :IP where account = :account");
                    qr.bindValue(":account", tempfriend.account);
                    qr.bindValue(":IP", Reply);    // IP地址 IP
                    qr.exec();
//                    query.bindValue(4, Reply);  // IP
//                    query.bindValue(5, ONLINE); // status
//                    query.exec();
                    tempfriend.node.hostAddr = Reply;
                    tempfriend.status = ONLINE;
                    qDebug() << "好友" << tempfriend.account << "在线，IP地址: " << Reply;
//                    replyKind = FRIEND_ONLINE;
//                    friendInfo.status = ONLINE;
//                    friendInfo.node.hostAddr = Reply;       // IP 地址为回复信息
//                    friendInfo.node.hostPort = getPortNumber(friendInfo.account);   // 端口号为 账号后4位数字
                }
                else if(Reply == "")
                {
//                    replyKind = NO_REPLY;
//                    friendInfo.status = OFFLINE;
                    allReplyed = false;
                }
                else
                {
//                    replyKind = FRIEND_NO_ACCOUNT;
//                    friendInfo.status = OFFLINE;
                    allReplyed = false;
                }

            }
            else
            {
                allReplyed = false; // 只要有一个回复未收到或者超时即未完全回复
            }
            if(tempfriend.account == loginInfo.account)    // 如果数据库中好友为本身
            {
                friendVect.push_front(tempfriend); // 在最前端的好友中加自己 friendVect[0] 默认为自己
                selfincluded = true;
    //            friendVect.insert(friendVect.begin(), tempfriend);    //
            }
            else
            {
                friendVect.push_back(tempfriend);  // 默认其他好友添加至最后
            }
        }
        if(false == selfincluded)       // 数据库中未包含自身，另行添加
        {
            FriendInfo myself;
            myself.account = loginInfo.account;
            myself.name = loginInfo.nickname;
            myself.avatar = loginInfo.avatar;
            friendVect.push_front(myself); // 在最前端插入自己为好友
            // 在数据库中做填补工作
            query.prepare("INSERT INTO friends (ID, account, nickname, avatar, IP, status, block, group) VALUES (:ID, :account, :nickname, :avatar, :IP, :status, :block, :group)");
            query.bindValue(":account", myself.account);
            query.bindValue(":nickname", myself.name);
            query.bindValue(":avatar", myself.avatar);
            bool bsuccess = query.exec();
            if(false == bsuccess)
            {
                QString queryError = query.lastError().text();
                qDebug() << "插入个人信息不成功，错误代码：" << queryError;
            }
            else
            {
                qDebug() << "成功插入个人信息";
            }

        }

    }
    return allReplyed;

}

/// 与好友（服务器）通信，向好友发送请求
// 添加好友请求
void TCPLink::addFriendRequest()
{
    requestKind = ADD_FRIEND;
    TCPLink::newTCPConnection();
}
