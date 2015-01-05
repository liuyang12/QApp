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
//    FriendInfo myself;
//    myself.account = loginInfo.account;
//    myself.status = loginInfo.status;
//    friendVect.push_back(myself);  // myself = friendVect[0]，第 0 位好友为其本身
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
// 重新连接好友消息信号和槽
void TCPLink::reconnectfriendSocket()
{
    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
    // 获取发送的数据信息
    /** friendInfo.tcpSocket = tcpSender */
    connect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendData()));     // 已连接向服务器发送请求
    connect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
}
 // 在 friendVect[] 中查找相应的账号，如果存在则已经是好友，返回序号，如果不存在则还不是好友返回 -1
int TCPLink::findAccount(QString &account)
{
    for(int i = 0; i < friendVect.size(); i++)
    {
        if(friendVect[i].account == account)
            return i;
    }
    return -1;
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
        TCPLink::reconnectfriendSocket();   // 只要 friendInfo.tcpSocket 改变，重新连接信号和槽
        friendInfo.node.hostAddr = friendInfo.tcpSocket->peerAddress().toString();   // 获取客户端IP地址
        //    friendInfo.node.hostPort = friendInfo.tcpSocket->peerPort();  // 获取客户端端口号
        qDebug() << "accept connection from client: " << friendInfo.node.hostAddr;
        friendInfo.isConnected = true;  // 已连接
//        if(friendInfo.tcpSocket->waitForReadyRead(500)) // 等待接收客户端的标识信息
//        {
//            QByteArray qba;
//            QString Reply;
//            qba = friendInfo.tcpSocket->readAll();
//            Reply = QVariant(qba).toString();
//            qDebug() << Reply;
//            if(Reply.left(8) == "connect_")     // 收到的好友回复信息为 "connect_$account"
//            {
//                QString friendAccount;
//                friendAccount = Reply.right(10);    // 好友账号为右边十位
//                int friendNumber = findAccount(friendAccount);
//                if(-1 == friendNumber)    // 还不是好友
//                {
//                    friendInfo.account = friendAccount;
//                    friendInfo.status = ONLINE;
//                    emit newReplySignal(ADDFRIEND_REQUEST); // 发出接收到请求加为好友的请求
//                    // 当tcpSocket在接受客户端连接时出现错误时，displayError(QAbstractSocket::SocketError) 处理该信号
//                    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
//                    // 获取发送的数据信息
//                    /** friendInfo.tcpSocket = tcpSender */
//                    connect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendData()));     // 已连接向服务器发送请求
//                    connect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
//                }
//                else    // 已经是好友了 i，开始聊天
//                {
//                    friendVect[friendNumber].tcpSocket = friendInfo.tcpSocket;      // 将这个 TCPSocket 与该好友绑定
//                    friendInfo.tcpSocket = new QTcpSocket;  // 为 friendInfo new 一个新的 TCPSocket
//                    friendInfo.tcpSocket = false;
//                    friendVect[friendNumber].isConnected = true;
//                    replyKind = STARTCHAT_REQUEST;
//                    friendInfo = friendVect[friendNumber];  // 作临时赋值
//                    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
//                    // 获取发送的数据信息
//                    /** friendInfo.tcpSocket = tcpSender */
//                    connect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendData()));     // 已连接向服务器发送请求
//                    connect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
//                //    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));    // 处理错误信息
//                    emit newReplySignal(replyKind); // 接收到好友聊天请求
//                }

//            }

//        }
//        connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
//        // 获取发送的数据信息
//        /** friendInfo.tcpSocket = tcpSender */
//        connect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendData()));     // 已连接向服务器发送请求
//        connect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
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
//    QStringList strList;
    int friendNumber;
    friendNumber = findAccount(friendInfo.account);
    if(friendNumber > -1)
    {
        friendInfo.isConnected = friendVect[friendNumber].isConnected;
        if(friendVect[friendNumber].isConnected)
        {
            friendInfo.tcpSocket = friendVect[friendNumber].tcpSocket;
            TCPLink::reconnectfriendSocket();       // 只要 friendInfo.tcoSocket 变化就重新连接
        }
    }

    if(friendNumber != 0 && friendInfo.isConnected == false)
    {
        ////
        friendInfo.tcpSocket->abort();  // 解决为什么经常在这里出现 bug
        friendInfo.tcpSocket->connectToHost(friendInfo.node.hostAddr, getPortNumber(friendInfo.account) /*friendInfo.node.hostPort*/); // 连接到 待加好友服务器，IP地址为回复，端口号为好友账号后四位
        qDebug() << "connect to host: IP " << friendInfo.node.hostAddr << " port " << getPortNumber(friendInfo.account);
        if(friendInfo.tcpSocket->waitForConnected(500)) // 等待 500ms发送请求
            TCPLink::sendData();
//        if(-1 == friendNumber)  // 还不是好友，需要接下来发送好友请求
//        {
//            return ;
//        }
//        if(ADD_FRIEND == requestKind)
//            requestKind = 0;    // 避免再次发送好友请求消息
//        if(friendInfo.tcpSocket->waitForConnected(500)) // 等待与好友服务器建立连接 500ms超时
//        {
//            QString connectStr;
//            connectStr = "connect_" + loginInfo.account;
//            friendInfo.tcpSocket->write(connectStr.toStdString().c_str());  // 连接至好友服务器，并发送 "connect_$account" 作标识，对方（服务器端）同时等待接收
//            qDebug() << connectStr;
//            friendInfo.isConnected = true;  // 已经建立与好友的连接
//            if(-1 == friendNumber)  // 还不是好友，需要接下来发送好友请求，相当于已经发送了好友请求
//            {
//                if(ADD_FRIEND == requestKind)
//                    requestKind = 0;
//                return ;
//            }
//            friendVect[friendNumber].tcpSocket = friendInfo.tcpSocket;  // 将这个 tcpSocket 赋予该好友
////            friendInfo.tcpSocket = new QTcpSocket;  // 重新建立新的TCPSocket
////            friendInfo.isConnected = false;
//            friendInfo = friendVect[friendNumber];
//            connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
//            // 获取发送的数据信息
//            /** friendInfo.tcpSocket = tcpSender */
//            connect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendData()));     // 已连接向服务器发送请求
//            connect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
//        //    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));    // 处理错误信息

//            friendVect[friendNumber].isConnected = true;
//            if(ADD_FRIEND == requestKind)
//                requestKind = 0;
//            else if(STARTCHAT_REQUEST == requestKind)   // 如果请求开始聊天，直接开始聊天
//            {
//                replyKind = STARTCHAT_SUCCESS;
//                emit newReplySignal(replyKind);
//            }
//            else
//                TCPLink::sendData();
//        }

////        friendInfo.isConnected = true;
    }
    else
    {
//        if(-1 == friendNumber || START_CHAT == requestKind)
//        if(requestKind != CONNECT)
            TCPLink::sendData();    // 只有还不是好友，才发送好友请求
    }

}
// 发送请求
void TCPLink::sendData()
{
    int friendNumber;
    friendNumber = findAccount(friendInfo.account);
    switch (requestKind) {
    case ADD_FRIEND:       // 发送好友请求 "add_$account"
        qDebug() << tr("add_")<<loginInfo.account;
        friendInfo.tcpSocket->write(QString("add_"+loginInfo.account).toStdString().c_str());
        break;
    case START_CHAT:        // 发出聊天请求 "chat_$account"
        qDebug() << tr("chat_") << loginInfo.account;
        friendInfo.tcpSocket->write(QString("chat_"+loginInfo.account).toStdString().c_str());
        friendVect[friendNumber].tcpSocket = friendInfo.tcpSocket;
        friendVect[friendNumber].isConnected = true;    // 已经连接

        break;
    case CONNECT:           // 发出连接请求 "connect_$account"
        friendInfo.tcpSocket->write(QString("connect_"+loginInfo.account).toStdString().c_str());
        qDebug() << tr("connect_") << loginInfo.account;
        friendVect[friendNumber].tcpSocket = friendInfo.tcpSocket;
        friendVect[friendNumber].isConnected = true;    // 已经连接

        break;
    case GROUP_CHAT:        // 发出群聊请求 "groupchat_$account_$1_$2_$3"
        friendInfo.tcpSocket->write(QString("groupchat_"+groupString).toStdString().c_str());
        qDebug() << tr("groupchat_") << groupString;
        friendVect[friendNumber].tcpSocket = friendInfo.tcpSocket;
        friendVect[friendNumber].isConnected = true;    // 已经连接
//    case :

//        break;
    default:
        break;
    }
    friendInfo.isConnected = true;  // 已经建立与好友的连接
}
// 读取请求
void TCPLink::recieveData()
{
   QByteArray qba;
   QString Reply;
   int friendNumber;
   qba = friendInfo.tcpSocket->readAll();
   Reply = QVariant(qba).toString();
   qDebug() << Reply;
   if(Reply.left(10) == "groupchat_")       // 好友发出群聊请求
   {
       groupString = Reply.remove(0, 10);  // 删除前十个字符 "groupchat_"
       qDebug() << groupString;
       if((groupString.size()+1) % 11 == 0)     // 如果长度+1是11的整数倍，那么有这么多好友，否则不是群聊
       {
           QStringList accountList;
           accountList = groupString.split("_");      // 按 "_" 截取好友
           friendInfo.account = accountList[0];         // 发起请求的是第一个好友
           friendNumber = findAccount(friendInfo.account);
           if(-1 == friendNumber)
           {
               qDebug() << "用户" << friendInfo.account << "还不是您的好友\n别忘了返回添加哦~";
           }
           else if(0 == friendNumber)
           {
               qDebug() << "亲爱的" << friendVect[findAccount(loginInfo.account)].name << "，目前还不支持同一账号多客户端登陆哦~\n您可以等待我们的更新，谢谢！";
           }
           else
           {
               friendVect[friendNumber].tcpSocket = friendInfo.tcpSocket;
               friendVect[friendNumber].isConnected = friendInfo.isConnected;
               friendVect[friendNumber].node.hostAddr = friendVect[friendNumber].node.hostAddr;
           }
           replyKind = GROUPCHAT_REQUEST;       // 群聊请求
       }
       else
       {
           qDebug() << "群聊消息不正确";
       }

   }
   else if(Reply.left(8) == "connect_")  // 好友发出连接请求
   {
        friendInfo.account = Reply.right(10);
        friendNumber = findAccount(friendInfo.account);
        if(-1 == friendNumber)
        {
            qDebug() << "用户" << friendInfo.account << "还不是您的好友\n别忘了返回添加哦~";
        }
        else if(0 == friendNumber)
        {
            qDebug() << "亲爱的" << friendVect[findAccount(loginInfo.account)].name << "，目前还不支持同一账号多客户端登陆哦~\n您可以等待我们的更新，谢谢！";
        }
        else
        {
            friendVect[friendNumber].tcpSocket = friendInfo.tcpSocket;
            friendVect[friendNumber].isConnected = friendInfo.isConnected;
            friendVect[friendNumber].node.hostAddr = friendVect[friendNumber].node.hostAddr;
        }
        replyKind = CONNECT_REQUEST;
   }
   else if(Reply == "connected")    // 好友已经连接
   {
       replyKind = CONNECT_SUCCESS;
   }
   else if(Reply.left(5) == "chat_") // 对方发起聊天请求
   {
       friendInfo.account = Reply.right(10);
       friendNumber = findAccount(friendInfo.account);
       if(-1 == friendNumber)
       {
           qDebug() << "用户" << friendInfo.account << "还不是您的好友\n别忘了返回添加哦~";
       }
       else if(0 == friendNumber)
       {
           qDebug() << "亲爱的" << friendVect[0].name << "，目前还不支持同一账号多客户端登陆哦~\n您可以等待我们的更新，谢谢！";
       }
       else
       {
           friendVect[friendNumber].tcpSocket = friendInfo.tcpSocket;
           friendVect[friendNumber].isConnected = friendInfo.isConnected;
           friendVect[friendNumber].node.hostAddr = friendVect[friendNumber].node.hostAddr;
       }
       replyKind = STARTCHAT_REQUEST;   // 发起聊天请求
   }
   else if(Reply == "start_chat")
   {
       replyKind = STARTCHAT_SUCCESS;
   }
   else if(Reply == "cancel_chat")
   {
       replyKind = STARTCHAT_DENY;
   }
   else if(Reply.left(4) == "add_"/*accountRegExp.exactMatch(Reply)*/)       // 回复匹配学号
   {
//       chatWindow *chat;
       friendInfo.account = Reply.right(10);
//       friendInfo.node = friendInfo.tcpSocket-
       replyKind = ADDFRIEND_REQUEST;

   }
   else if(Reply == "ADD")  // 好友接收好友请求
   {
//       chatWindow *chat;
//       friendVect.push_back(friendInfo);
//       databaseInsert(friendInfo);
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
    TCPLink::reconnectfriendSocket();       // 只要更新了 friendInfo.tcpSocket 就重新连接
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
// 查询单个好友在线状态，并反馈至数据库中
int TCPLink::confirmFriendOnline(QString &account)
{
    int friendNumber = findAccount(account);    // 好友在好友列表中的序号
    if(-1 == friendNumber)
    {
        qDebug() << "尚未添加好友 " << account;
        return -2;
    }
    else if(0 == friendNumber)
    {
        qDebug() << "用户" << account << "目前不支持多地同时登陆，不能与自己聊天";
        return -2;
    }
    // friendNumber >= 1
    if(!isConnected)
    {
        tcpClient->abort();
        tcpClient->connectToHost(serverNode.hostAddr, serverNode.hostPort);
    }
    else
    {
        QString QueryInfo;
        QueryInfo = "q" + account;
        qDebug() << QueryInfo;
        tcpClient->write(QueryInfo.toStdString().c_str());
        disconnect(tcpClient, SIGNAL(readyRead()), this, SLOT(readResult()));   // 暂时 disconnect
        if(tcpClient->waitForReadyRead(500))    // 等待数据可读，500ms超时
        {
            isConnected = true;
            QSqlQuery qr;
            QByteArray qba;
            QString Reply;
            qba = tcpClient->readAll();
            Reply = QVariant(qba).toString();
            qDebug() << Reply;
            if(Reply == "n")
            {
                // 好友不在线
                qr.prepare("update friends set status = :status where account = :account");
                qr.bindValue(":account", account);
                qr.bindValue(":status", OFFLINE);    // 在线状态 status
                qr.exec();
//                    query.exec();
                qDebug() << "好友" << account << "离线";
//                    replyKind = FRIEND_OFFLINE;
//                    friendInfo.status = OFFLINE;
                // 同时更新 friendVect[]
                friendVect[friendNumber].node.hostAddr = "";    // 用户不在线，在线IP清零
                friendVect[friendNumber].status = OFFLINE;
                friendVect[friendNumber].isConnected = false;   // 不在线，未连接
                emit friendstatusChangedSignal();       // 发出好友在线状态改变信号
                return OFFLINE;

            }
            else if(ipRegExp.exactMatch(Reply))
            {
                qr.prepare("update friends set status = :status where account = :account");
                qr.bindValue(":account", account);
                qr.bindValue(":status", ONLINE);    // 在线状态 status
                qr.exec();
                qr.prepare("update friends set IP = :IP where account = :account");
                qr.bindValue(":account", account);
                qr.bindValue(":IP", Reply);    // IP地址 IP
                qr.exec();
//                    query.bindValue(4, Reply);  // IP
//                    query.bindValue(5, ONLINE); // status
//                    query.exec();
//                tempfriend.node.hostAddr = Reply;
//                tempfriend.status = ONLINE;
                qDebug() << "好友" << account << "在线，IP地址: " << Reply;
//                    replyKind = FRIEND_ONLINE;
//                    friendInfo.status = ONLINE;
//                    friendInfo.node.hostAddr = Reply;       // IP 地址为回复信息
//                    friendInfo.node.hostPort = getPortNumber(friendInfo.account);   // 端口号为 账号后4位数字
                // 同时更新 friendVect[]
                friendVect[friendNumber].status = ONLINE;
                if(friendVect[friendNumber].node.hostAddr != Reply) // IP 地址变化，需要重新创建连接
                {
                    friendVect[friendNumber].node.hostAddr = Reply;
                    friendVect[friendNumber].isConnected = false;   // 不再处于连接状态，重新连接
//                    return IPUPDATED;
                }
                emit friendstatusChangedSignal();       // 发出好友在线状态改变信号
                return ONLINE;
            }
            else if(Reply == "")
            {
//                    replyKind = NO_REPLY;
//                    friendInfo.status = OFFLINE;
//                allReplyed = false;
                return -1;
            }
            else
            {
//                    replyKind = FRIEND_NO_ACCOUNT;
//                    friendInfo.status = OFFLINE;
//                allReplyed = false;
                return -1;
            }

        }

        connect(tcpClient, SIGNAL(readyRead()), this, SLOT(readResult()));   // 恢复 disconnect

    }
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(readResult()));   // 恢复 disconnect
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
                isConnected = true;
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
//                    emit friendstatusChangedSignal();       // 发出好友状态改变信号
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
//            if(tempfriend.account == loginInfo.account)    // 如果数据库中好友为本身
//            {
//                friendVect.push_front(tempfriend); // 在最前端的好友中加自己 friendVect[0] 默认为自己
//                selfincluded = true;
//    //            friendVect.insert(friendVect.begin(), tempfriend);    //
//            }
//            else
//            {
//                friendVect.push_back(tempfriend);  // 默认其他好友添加至最后
//            }
        }
//        if(false == selfincluded)       // 数据库中未包含自身，另行添加
//        {
//            FriendInfo myself;
//            myself.account = loginInfo.account;
//            myself.name = loginInfo.nickname;
//            myself.avatar = loginInfo.avatar;
//            friendVect.push_front(myself); // 在最前端插入自己为好友
//            // 在数据库中做填补工作
//            query.prepare("INSERT INTO friends (ID, account, nickname, avatar, IP, status, block, group) VALUES (:ID, :account, :nickname, :avatar, :IP, :status, :block, :group)");
//            query.bindValue(":account", myself.account);
//            query.bindValue(":nickname", myself.name);
//            query.bindValue(":avatar", myself.avatar);
//            bool bsuccess = query.exec();
//            if(false == bsuccess)
//            {
//                QString queryError = query.lastError().text();
//                qDebug() << "插入个人信息不成功，错误代码：" << queryError;
//            }
//            else
//            {
//                qDebug() << "成功插入个人信息";
//            }

//        }

    }
    return allReplyed;

}
// 将 friendVect[] 与数据库的信息同步
void TCPLink::databasetoFriendVect()
{
    QSqlQuery query;    // 数据库查询
    bool selfincluded;
    selfincluded = false;
    query.exec("select * from friends");    // 指定查找数据库中 friends 表

    while(query.next()) // 遍历整张表
    {
        FriendInfo tempfriend;
        // 0 ID
        tempfriend.account = query.value(1).toString();     // 1 账号
        tempfriend.name = query.value(2).toString();        // 2 昵称
        tempfriend.avatar = query.value(3).toString();      // 3 头像路径，相对路径
        tempfriend.node.hostAddr = query.value(4).toString();// 4 IP
        tempfriend.status = query.value(5).toInt();      // 5 status
        // 4 IP
        // 5 status
        tempfriend.block = query.value(6).toString();       // 6 所在分组
        tempfriend.group = query.value(7).toString();       // 7 所在群

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
//        query.prepare("insert into friends (ID, account, nickname, avatar, IP, status, block, group) values (:ID, :account, :nickname, :avatar, :IP, :status, :block, :group)");
        query.prepare("insert into friends (account, nickname, avatar) values (:account, :nickname, :avatar)");
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
// 向数据库中插入一个新的好友信息
void TCPLink::databaseInsert(FriendInfo &frd)
{
    QSqlQuery qr;
    // 插入一个新的好友，并更新相应的信息
    qr.prepare("insert into friends (account, nickname, avatar, IP, status) values(:account, :nickname, :avatar, :IP, :status)");
    qr.bindValue(":accout", frd.account);   // 账号
    qr.bindValue(":nickname", frd.name);    // 昵称
    qr.bindValue(":avatar", frd.avatar);    // 头像路径
    qr.bindValue(":IP", frd.node.hostAddr);// IP
    qr.bindValue(":status", frd.status);    // 状态 status
//    qr.bindValue(":block", ); // block 分组
//    qr.bindValue(":group", ); // group 群
    qr.exec();
}

/// 与好友（服务器）通信，向好友发送请求
// 添加好友请求
void TCPLink::addFriendRequest()
{
    requestKind = ADD_FRIEND;
    TCPLink::newTCPConnection();
}
// 好友聊天请求
void TCPLink::startChatRequest()
{
    requestKind = START_CHAT;
    TCPLink::newTCPConnection();
}
// 建立连接请求
void TCPLink::connectRequest()
{
    requestKind = CONNECT;
    TCPLink::newTCPConnection();
}
// 群聊请求
void TCPLink::groupChatRequest(QString groupStr)
{
    requestKind = GROUP_CHAT;
    groupString = groupStr;
    TCPLink::newTCPConnection();
}

/// 群聊
// 群聊请求
void TCPLink::startGroupChat(QVector<int> friendNo)
{
    // 之前应该建立与每一位好友的连接
    FriendInfo tmpfriend;   // 临时转存
//    QString groupChatStr;
    groupString = /*"groupchat_"*/loginInfo.account;
    tmpfriend = friendInfo;    // 首先保存临时好友信息
    qDebug() << friendNo;
    for(int i = 0; i < friendNo.size(); i++)
    {
        groupString = groupString + "_" + friendVect[friendNo[i]].account;    // "$account_$1_$2_$3"
    }
    for(int i = 0; i < friendNo.size(); i++)
    {
        // 每次打开聊天窗口确定所有好友的在线状态
        if(ONLINE == confirmFriendOnline(friendVect[friendNo[i]].account) || IPUPDATED == confirmFriendOnline(friendVect[friendNo[i]].account))  // 在线
        {
            if(!friendVect[friendNo[i]].isConnected)   // 如果还未连接则进行连接
            {
                friendInfo = friendVect[friendNo[i]];
                groupChatRequest(groupString); // 向每一位好友发出群聊请求
            }
        }
    }
//    qDebug() << groupChatStr;
    // 向每一位好友发出群聊请求
//    for(int i = 0; i < friendNo.size(); i++)
//    {
//        if(ONLINE == friendVect[friendNo[i]].status)    // 好友在线，即有相应的 TCPSocket
//        {
//            /// TODO：给每一在线好友加消息 "groupchat_$account"
//            requestKind = GROUP_CHAT;
//            friendVect[friendNo[i]].tcpSocket->write(groupChatStr.toStdString().c_str());
//            ///
//        }
//    }
    friendInfo = tmpfriend;    // 恢复临时好友信息
}
