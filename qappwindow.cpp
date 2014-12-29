#include "qappwindow.h"
#include "ui_qappwindow.h"
#include "chatwindow.h"     // 聊天窗口
#include <QString>
#include <QStringList>
#include "tcplink.h"

extern TCPLink *tcplink;           // tcplink 全局变量

QAppWindow::QAppWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QAppWindow)/*,
    tcplink(tcplink)*/
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);     // 设置窗口无边框
    ui->closeButton->setMouseTracking(true);
    ui->closeButton->setStyleSheet("QPushButton{border-image: url(:/mainpicture/kb.png);background-image: url(:/mainpicture/kb.png);}"
                                    "QPushButton:hover{border-image: url(:/mainpicture/kb.png);background-color: qlineargradient(spread:pad, x1:0.555682, y1:0.222, x2:0.55, y2:1, stop:0.482955 rgba(213, 0, 10, 232), stop:1 rgba(213, 0, 10, 82));}"
                                    );
    ui->qappminButton->setMouseTracking(true);
    ui->qappminButton->setStyleSheet("QPushButton{border-image: url(:/mainpicture/kb.png);background-image: url(:/mainpicture/kb.png);}"
                                    "QPushButton:hover{border-image: url(:/mainpicture/kb.png);background-color: qlineargradient(spread:pad, x1:0.555682, y1:0.222, x2:0.55, y2:1, stop:0.482955 rgba(213, 0, 10, 232), stop:1 rgba(213, 0, 10, 82));}"
                                    );
    ui->Button_queryFriend->setMouseTracking(true);
    ui->Button_queryFriend->setStyleSheet("QPushButton{border-image: url(:/mainpicture/find_normal.png);background-image: url(:/mainpicture/kb.png);}"
                                          "QPushButton{border-image: url(:/mainpicture/find_hover.png);background-image: url(:/mainpicture/kb.png);}"
                                          );
    //设置加好友输入框不显示
    flag = false;
    ui->Edit_FriendAccount->setVisible(flag);
    ui->Button_queryFriend->setVisible(flag);

    tcplink->initasServer();
    QAppWindow::initStatus();   // 初始状态设置

    //监听初始化
    tcpServer = new QTcpServer();
    if(!tcpServer->listen(QHostAddress::Any,16666))
    {
        qDebug() << tcpServer->errorString();
        close();
    }
    ReceiveFile.EachSize = 0;
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));

    //建树
    build_tree();
    ui->treeWidget->setFrameStyle(QFrame::NoFrame);
}

QAppWindow::~QAppWindow()
{
    delete ui;
}

//拖动窗口
void QAppWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
void QAppWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() && Qt::LeftButton)
    {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}



// 主界面初始状态设置
void QAppWindow::initStatus()
{
//    FriendInfo myself;
//    myself.account = tcplink->loginInfo.account;
//    myself.status = tcplink->loginInfo.status;
//    tcplink->friendVect.push_back(myself);  // myself = friendVect[0]，第 0 位好友为其本身
    // 主界面初始状态设置
    ui->Edit_FriendAccount->setText("2012011");
//    tcplink->userInfo.account = tcplink->loginInfo.account;
//    tcplink->userInfo.status = tcplink->loginInfo.status = ONLINE;

    connect(ui->Edit_FriendAccount, SIGNAL(returnPressed()), this, SLOT(on_Button_queryFriend_clicked()));
    // 接收TCP Socket 通信返回信息信号与 this 的槽相连
    connect(tcplink, SIGNAL(newReplySignal(qint32)), this, SLOT(newReply(qint32)));
    connect(tcplink, SIGNAL(connectionFailedSignal()), this, SLOT(initStatus()));
    connect(tcplink, SIGNAL(disconnectedSignal()), this, SLOT(serverDisconnected()));

//    timer = new QTimer;
//    this->tcpServer = new QTcpServer(this);
////    this->tcpSender = new QTcpSocket(this);
//    tcplink->friendInfo.tcpSocket = new QTcpSocket(this);

//    newListen();    // 监听客户端请求。端口号 为学号后四位数字
//    // 当有客户端访问时，发出newConnection() 信号，acceptConnection() 处理该新建连接信号
//    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
//    // 当tcpSocket在接受客户端连接时出现错误时，displayError(QAbstractSocket::SocketError) 处理该信号
//    connect(tcplink->friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
//    // 获取发送的数据信息
//    /** tcplink->friendInfo.tcpSocket = tcpSender */
//    connect(tcplink->friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendRequest()));     // 已连接向服务器发送请求
//    connect(tcplink->friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
////    connect(tcplink->friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));    // 处理错误信息


}
//// 监听客户端请求，端口号 为学号后四位数字，以
//void QAppWindow::newListen()
//{
//    // 监听是否有客户端来访，且对任意来访者进行监听，端口号 为学号后四位数字
//    if(!tcpServer->listen(QHostAddress::Any, /*6666*/getPortNumber(tcplink->loginInfo.account)))
//    {
//        qDebug() << tcpServer->errorString();
//        close();
//        return ;
//    }
//}
//// 当有客户端访问时，发出newConnection() 信号，acceptConnection() 处理该新建连接信号
//void QAppWindow::acceptConnection()
//{
//    // 当有客户来访时，将tcpSocket接受tcpServer建立的socket
//    tcplink->loginInfo.request = GET_FRIEND;
//    tcplink->friendInfo.tcpSocket = tcpServer->nextPendingConnection();
//    tcplink->friendInfo.node.hostAddr = tcplink->friendInfo.tcpSocket->peerAddress().toString();   // 获取客户端IP地址
////    tcplink->friendInfo.node.hostPort = tcplink->friendInfo.tcpSocket->peerPort();  // 获取客户端端口号
//    qDebug() << "accept connection from client: " << tcplink->friendInfo.node.hostAddr;
//    // 当tcpSocket在接受客户端连接时出现错误时，displayError(QAbstractSocket::SocketError) 处理该信号
//    connect(tcplink->friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
//    // 获取发送的数据信息
//    /** tcplink->friendInfo.tcpSocket = tcpSender */
//    connect(tcplink->friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendRequest()));     // 已连接向服务器发送请求
//    connect(tcplink->friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(recieveData()));     // 准备读取服务器端的数据
////    connect(tcplink->friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));    // 处理错误信息
//}
//// 当tcpSocket在接受客户端连接时出现错误时，displayError(QAbstractSocket::SocketError) 处理该信号
//void QAppWindow::displayError(QAbstractSocket::SocketError)
//{
//    qDebug() << tcplink->friendInfo.tcpSocket->errorString()/*tcplink->friendInfo.tcpSocket->errorString()*/;
//    tcplink->friendInfo.tcpSocket->close();
//}
//// 建立连接到该好友的
//void QAppWindow::newTCPConnect()
//{
//    tcplink->friendInfo.tcpSocket->abort();
//    tcplink->friendInfo.tcpSocket->connectToHost(tcplink->friendInfo.node.hostAddr, getPortNumber(tcplink->friendInfo.account) /*tcplink->friendInfo.node.hostPort*/); // 连接到 待加好友服务器，IP地址为回复，端口号为好友账号后四位
//    qDebug() << "connect to host: IP " << tcplink->friendInfo.node.hostAddr << " port " << getPortNumber(tcplink->friendInfo.account);
//}
// 与服务器断开连接
void QAppWindow::serverDisconnected()
{
    // 处理与服务器断开连接的相关问题
    emit disconnectedSignal();
}
// 发送请求
//void QAppWindow::sendRequest()
//{

//    switch (tcplink->loginInfo.request) {
//    case SEND_FRIEND:       // 发送好友请求
//        qDebug() << tcplink->loginInfo.account.toStdString().c_str();
//        tcplink->friendInfo.tcpSocket->write(tcplink->loginInfo.account.toStdString().c_str());
//        break;
////    case :

////        break;
//    default:
//        break;
//    }
//}
//// 读取请求
//void QAppWindow::recieveData()
//{
//   QByteArray qba;
//   QString Reply;
//   qba = tcplink->friendInfo.tcpSocket->readAll();
//   Reply = QVariant(qba).toString();
//   qDebug() << Reply;
//   if(Reply.size() == 10/*accountRegExp.exactMatch(Reply)*/)       // 回复匹配学号
//   {
//       chatWindow *chat;
//       tcplink->friendInfo.account = Reply;
////       tcplink->friendInfo.node = tcplink->friendInfo.tcpSocket-
//       switch (QMessageBox::information(this,tr("好友请求"), tr("用户")+Reply+tr("请求加为好友？\n是否加为好友"), "加为好友(&A)", "取消(&C)", 0)) {
//       case 0:  // 加为好友 - 默认
//           tcplink->friendInfo.tcpSocket->write(tr("ADD").toStdString().c_str());       // 发送接收好友请求
//           tcplink->friendVect.push_back(tcplink->friendInfo);  // 添加好友
////           tcplink->friendInfo = new
//           /** 显示在主界面窗口上 */
//           chat = new chatWindow;       // 成功添加好友，打开聊天窗口
//           chat->show();
//           break;
//       case 1:
//           tcplink->friendInfo.tcpSocket->write(tr("CANCEL").toStdString().c_str());    // 发送取消好友请求
//           break;
//       default:
//           break;
//       }
//   }
//   else if(Reply == "ADD")  // 好友接收好友请求
//   {
//       chatWindow *chat;
//       tcplink->friendVect.push_back(tcplink->friendInfo);
//       /** 将好友信息显示在主界面窗口上 */
//       chat = new chatWindow;
//       chat->show();

//   }
//   else if(Reply == "CANCEL")// 好友取消好友请求
//   {

//   }
//}

// 收到新回复 - 服务器或者好友服务器
void QAppWindow::newReply(qint32 replyKind)
{
    QMessageBox friendMsgBox;
    chatWindow *chat;

    switch (replyKind) {
    /** 查找好友结果 */
    case FRIEND_ONLINE:
        qDebug() << "已找到用户" << tcplink->friendInfo.account << "IP地址: " << tcplink->friendInfo.node.hostAddr;


        switch(friendMsgBox.question( this, "查找好友",
                                      "好友在线\n"
                                      "现在加为好友~",
                                      "加为好友(&A)", "取消(&C)",
                                      0     /* Enter == button 0*/
                                      ))
        { // Escape == button 1
        case 0: // “加为好友”或者 ALT + S 按下被
            // 发出加为好友的请求
            friendMsgBox.setWindowTitle("loading。。。");
            friendMsgBox.setText("正在发出好友请求\n"
                                 "等待加为好友。。。");
            tcplink->addFriendRequest();
            //tcplink->friendInfo.tcpSocket = tcpSender;

//            chat = new chatWindow;
//            chat->show();
            friendMsgBox.close();

            break;
        case 1: // Cancel被点击或者Alt+C被按下或者Escape被按下。
            // 不退出
            break;
        }
        break;
    case FRIEND_OFFLINE:
        qDebug() << "用户" << tcplink->friendInfo.account << "离线";
        QMessageBox::information(this, tr("info"), tr("用户离线"), QMessageBox::Ok);
        break;
    case FRIEND_NO_ACCOUNT:
        qDebug() << "用户" << tcplink->friendInfo.account << "查找结果：查找账户错误";
        QMessageBox::information(this, tr("info"), tr("查找账户错误"), QMessageBox::Ok);
        break;
    case NO_REPLY:
        qDebug() << "用户" << tcplink->friendInfo.account << "查找结果：未收到服务器回复";
        QMessageBox::information(this, tr("info"), tr("未收到回复"), QMessageBox::Ok);
        break;
    /** 登出结果 */
    case LOGOUT_SUCCESS:
        qDebug() << "用户" << tcplink->loginInfo.account << "下线成功";
        QMessageBox::information(this, tr("info"), tr("下线成功"), QMessageBox::Ok);
        // 成功下线，关闭主界面，重新登录
        emit reLoginSignal();
        this->close();
        break;
    case LOGOUT_FAILED:
        qDebug() << "用户" << tcplink->loginInfo.account << "离线状态";
        QMessageBox::information(this, tr("info"), tr("离线状态"), QMessageBox::Ok);
        break;
    /** 好友请求 */
    case ADDFRIEND_REQUEST:
        switch (QMessageBox::information(this,tr("好友请求"), tr("用户")+tcplink->friendInfo.account+tr("请求加为好友？\n是否加为好友"), "加为好友(&A)", "取消(&C)", 0)) {
        case 0:  // 加为好友 - 默认
            tcplink->friendInfo.tcpSocket->write(tr("ADD").toStdString().c_str());       // 发送接收好友请求
            tcplink->friendVect.push_back(tcplink->friendInfo);  // 添加好友
 //           friendInfo = new
            /** 显示在主界面窗口上 */
            chat = new chatWindow;       // 成功添加好友，打开聊天窗口
            chat->setWindowTitle(tcplink->friendInfo.account);
            connect(this,SIGNAL(FriendInfoSignal(FriendInfo)),chat,SLOT(GetFriendInfo(FriendInfo)));
            chat->show();
            emit this->FriendInfoSignal(tcplink->friendInfo);
//            replyKind = ADDFRIEND_SUCCESS;   // 成功添加好友
            break;
        case 1:
            tcplink->friendInfo.tcpSocket->write(tr("CANCEL").toStdString().c_str());    // 发送取消好友请求
 //           replyKind = ADDFRIEND_DENY;      // 好友拒绝
            break;
        default:
            break;
        }
        break;
    case ADDFRIEND_SUCCESS: // 成功添加好友
        chat = new chatWindow;
        chat->setWindowTitle(tcplink->friendInfo.account);
        connect(this,SIGNAL(FriendInfoSignal(FriendInfo)),chat,SLOT(GetFriendInfo(FriendInfo)));
        chat->show();
        emit this->FriendInfoSignal(tcplink->friendInfo);
        break;
    case ADDFRIEND_DENY:    // 好友拒绝添加好友请求

        break;
    default:
        break;
    }
}

// 查找好友请求
void QAppWindow::on_Button_queryFriend_clicked()
{
    if(ui->Edit_FriendAccount->text().isEmpty() || ui->Edit_FriendAccount->text() == "2012011")
    {
        QMessageBox::critical(this, tr("ERROR"), tr("好友账号不能为空"), QMessageBox::Ok);
        return ;
    }
    tcplink->friendInfo.account = ui->Edit_FriendAccount->text();
//    tcplink->requestKind = QUERY;
    tcplink->queryRequest(tcplink->friendInfo);
}
// 登出请求
void QAppWindow::on_action_Logout_triggered()
{
    tcplink->userInfo.status = tcplink->loginInfo.status = OFFLINE;
    tcplink->userInfo.account = tcplink->loginInfo.account;
//    tcplink->requestKind = LOGOUT;
    tcplink->logoutRequest(tcplink->userInfo);
}



//--------------------------------------------------------
void QAppWindow::acceptConnection()
{
    tcpClient = tcpServer->nextPendingConnection();
    connect(tcpClient,SIGNAL(readyRead()),this,SLOT(updateServerProgress()));
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),this,
            SLOT(displayError(QAbstractSocket::SocketError)));
    tcpServer->close();
}

void QAppWindow::updateServerProgress()
{
    QDataStream in(tcpClient);
    in.setVersion(QDataStream::Qt_5_3);
    if(ReceiveFile.FinishedBytes <= sizeof(qint64)*2)
    {
        if((tcpClient->bytesAvailable() >= sizeof(qint64)*2)
                && (ReceiveFile.EachSize == 0))
        {
            in >> ReceiveFile.WholeBytes >> ReceiveFile.EachSize;
            ReceiveFile.FinishedBytes += sizeof(qint64) * 2;
        }
        if((tcpClient->bytesAvailable() >= ReceiveFile.EachSize)
                && (ReceiveFile.EachSize != 0))
        {
            in >> ReceiveFile.FileName;
            ReceiveFile.FinishedBytes += ReceiveFile.EachSize;
            ReceiveFile.File = new QFile(ReceiveFile.FileName);
            if(!ReceiveFile.File->open(QFile::WriteOnly))
            {
                qDebug() << "open file error!";
                return;
            }
        }
        else return;
    }
    if(ReceiveFile.FinishedBytes < ReceiveFile.WholeBytes)

    {  //如果接收的数据小于总数据，那么写入文件
        ReceiveFile.FinishedBytes += tcpClient->bytesAvailable();
        ReceiveFile.Buffer = tcpClient->readAll();
        ReceiveFile.File->write(ReceiveFile.Buffer);
        ReceiveFile.Buffer.resize(0);
    }
    if(ReceiveFile.FinishedBytes == ReceiveFile.WholeBytes)
    {
        tcpClient->close();
        ReceiveFile.File->close();
    }
}

void QAppWindow::displayError(QAbstractSocket::SocketError) //错误处理

{
    qDebug() << tcpClient->errorString();
    tcpClient->close();
}

void QAppWindow::on_closeButton_clicked()
{
    this->close();
}

void QAppWindow::on_qappminButton_clicked()
{
    this->showMinimized();
}

void QAppWindow::on_addfriend_clicked()
{
    flag = !flag;
    ui->Edit_FriendAccount->setVisible(flag);
    ui->Button_queryFriend->setVisible(flag);

}

//建TreeWidget
void QAppWindow::build_tree()
{
    ui->treeWidget->setStyleSheet("QTreeWidget::item{height:40px}");
    ui->treeWidget->setIconSize(QSize(40,40));

    QTreeWidgetItem *group = new QTreeWidgetItem(ui->treeWidget,QStringList(QString("clssmates")));
    QTreeWidgetItem *friend1 = new QTreeWidgetItem(group,QStringList(QString(" weijie")));
    friend1->setIcon(0,QIcon(":/picture/tx1.jpg"));


    ui->treeWidget->expandAll(); //结点全部展开
}

