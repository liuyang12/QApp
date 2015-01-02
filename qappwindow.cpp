#include "qappwindow.h"
#include "ui_qappwindow.h"
#include "chatwindow.h"     // 聊天窗口
#include <QString>
#include <QStringList>
#include "tcplink.h"
#include <QVector>
#include <algorithm>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

extern TCPLink *tcplink;           // tcplink 全局变量
//extern QSqlDatabase *db;            // db 数据库

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
    if(!tcpServer->listen(QHostAddress::Any,16666)) //固定采用16666作服务器端口
    {
        qDebug() << tcpServer->errorString();
        close();
    }
    ReceiveFile.EachSize = 0;
    FileConnect = false;
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    //文件接收窗口及进度条
    ReceiveDialog = new QDialog(this);
    ServerProgressBar = new QProgressBar(ReceiveDialog);
    OkPushButton = new QPushButton(ReceiveDialog);
    pLayout = new QVBoxLayout;
    ReceiveDialog->setWindowTitle(tr("接收文件"));
    ReceiveDialog->setFixedSize(200,80);
    ServerProgressBar->setFixedSize(190,30);
    OkPushButton->setFixedSize(160,30);
    OkPushButton->setText(tr("正在接收"));
    pLayout->addWidget(ServerProgressBar);
    pLayout->addWidget(OkPushButton);
    ReceiveDialog->setLayout(pLayout);
    connect(OkPushButton,SIGNAL(clicked()),this,SLOT(on_OkPushButton_clicked()));


    // 从数据库拉取数据，并更新 friendVect[]
//    QAppWindow::fetchDatabase();
    for(int i = 0; i < 3; i++)
    {
        /// travelsalRequest() 更新好友在线状态和IP地址并写入数据库，同时更新 friendVect[] （以自身为friendVect[0]）
        if(tcplink->travelsalRequest())
        {
            qDebug() << "所有好友在线状态更新完成";
           break;
        }
    }
    for(int i = 0; i < tcplink->friendVect.size(); i++)
    {
        qDebug() << tcplink->friendVect[i];
    }

    //建树
    build_tree();
    ui->treeWidget->setFrameStyle(QFrame::NoFrame);


}

QAppWindow::~QAppWindow()
{
    tcpServer->close();
    delete ui;
}
// 从数据库拉取数据，并更新 friendVect[]
void QAppWindow::fetchDatabase()
{
    QSqlQuery query;    // 数据库查询
    query.exec("select * from friends");    // 指定查找数据库中 friends 表
    bool selfincluded = false;      // 自身是否包含在数据库中
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
        if(tempfriend.account == tcplink->loginInfo.account)    // 如果数据库中好友为本身
        {
            tcplink->friendVect.push_front(tempfriend); // 在最前端的好友中加自己 friendVect[0] 默认为自己
            selfincluded = true;
//            tcplink->friendVect.insert(tcplink->friendVect.begin(), tempfriend);    //
        }
        else
        {
            tcplink->friendVect.push_back(tempfriend);  // 默认其他好友添加至最后
        }
        tcplink->travelsalRequest(/*tempfriend.account*/);  // 遍历好友在线状态请求
    }
    if(false == selfincluded)       // 数据库中未包含自身，另行添加
    {
        FriendInfo myself;
        myself.account = tcplink->loginInfo.account;
        myself.name = tcplink->loginInfo.nickname;
        myself.avatar = tcplink->loginInfo.avatar;
        tcplink->friendVect.push_front(myself); // 在最前端插入自己为好友
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
// 好友遍历回复
void QAppWindow::travelsalReply(qint32 replyKind)
{
    QSqlQuery query;
    switch (replyKind) {    // 根据回复做出回应
    case FRIEND_ONLINE:
        qDebug() << "用户" << tcplink->travelsalFriend.account << "在线，IP地址: " << tcplink->travelsalFriend.node.hostAddr;
        // 根据 账号信息更新 IP 和在线状态 status
        query.prepare("UPDATE friends set IP = :IP status = :status where account = :account ");
        query.bindValue(":account", tcplink->travelsalFriend.account);  // 账号
        query.bindValue(":IP", tcplink->travelsalFriend.node.hostAddr); // IP地址
        query.bindValue(":status", ONLINE); // 在线状态
        if(!query.exec())
        {
            qDebug() << query.lastError().text();
        }
        else
        {
            qDebug() << "update successfully!";
        }
        break;
    case FRIEND_OFFLINE:
        qDebug() << "用户" << tcplink->travelsalFriend.account << "离线";
        // 根据 账号信息更新 IP 和在线状态 status
        query.prepare("UPDATE friends set status = :status where account = :account ");
        query.bindValue(":account", tcplink->travelsalFriend.account);  // 账号
//        query.bindValue(":IP", tcplink->travelsalFriend.node.hostAddr); // IP地址
        query.bindValue(":status", OFFLINE); // 在线状态
        if(!query.exec())
        {
            qDebug() << query.lastError().text();
        }
        else
        {
            qDebug() << "update successfully!";
        }
//        QMessageBox::information(this, tr("info"), tr("用户离线"), QMessageBox::Ok);
        break;
    case FRIEND_NO_ACCOUNT:
        qDebug() << "用户" << tcplink->travelsalFriend.account << "查找结果：查找账户错误";
//        QMessageBox::information(this, tr("info"), tr("查找账户错误"), QMessageBox::Ok);
        // 根据 账号信息更新 IP 和在线状态 status
        query.prepare("UPDATE friends set status = :status where account = :account ");
        query.bindValue(":account", tcplink->travelsalFriend.account);  // 账号
//        query.bindValue(":IP", tcplink->travelsalFriend.node.hostAddr); // IP地址
        query.bindValue(":status", OFFLINE); // 在线状态
        if(!query.exec())
        {
            qDebug() << query.lastError().text();
        }
        else
        {
            qDebug() << "update successfully!";
        }
        break;
    case NO_REPLY:
        qDebug() << "用户" << tcplink->travelsalFriend.account << "查找结果：未收到服务器回复";
//        QMessageBox::information(this, tr("info"), tr("未收到回复"), QMessageBox::Ok);
        // 根据 账号信息更新 IP 和在线状态 status
        query.prepare("UPDATE friends set status = :status where account = :account ");
        query.bindValue(":account", tcplink->travelsalFriend.account);  // 账号
//        query.bindValue(":IP", tcplink->travelsalFriend.node.hostAddr); // IP地址
        query.bindValue(":status", OFFLINE); // 在线状态
        if(!query.exec())
        {
            qDebug() << query.lastError().text();
        }
        else
        {
            qDebug() << "update successfully!";
        }
        break;
    default:
        break;
    }
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
    // 主界面初始状态设置
    ui->Edit_FriendAccount->setText("2012011");
    connect(ui->Edit_FriendAccount, SIGNAL(returnPressed()), this, SLOT(on_Button_queryFriend_clicked()));
    // 接收TCP Socket 通信返回信息信号与 this 的槽相连
    connect(tcplink, SIGNAL(newReplySignal(qint32)), this, SLOT(newReply(qint32)));
    connect(tcplink, SIGNAL(travelsalReplySignal(qint32)), this, SLOT(travelsalReply(qint32))); // 好友遍历回复
    connect(tcplink, SIGNAL(connectionFailedSignal()), this, SLOT(initStatus()));
    connect(tcplink, SIGNAL(disconnectedSignal()), this, SLOT(serverDisconnected()));

}
// 与服务器断开连接
void QAppWindow::serverDisconnected()
{
    // 处理与服务器断开连接的相关问题
    emit disconnectedSignal();
}
// 收到新回复 - 服务器或者好友服务器
void QAppWindow::newReply(qint32 replyKind)
{
    QMessageBox friendMsgBox;
    chatWindow *chat;

    switch (replyKind) {
    /** 查找好友结果 */
    case FRIEND_ONLINE:
        qDebug() << "已找到用户" << tcplink->friendInfo.account << "IP地址: " << tcplink->friendInfo.node.hostAddr;
//        tcplink->friendVect.

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
    qDebug() << "Accept Send File Request From : " << tcpClient->peerAddress().toString();
    ReceiveFile.EachSize = 0;
    ReceiveFile.TodoBytes = 0;
    ReceiveFile.FinishedBytes = 0;
    ReceiveFile.WholeBytes = 0;
    FileConnect = false;
    connect(tcpClient,SIGNAL(readyRead()),this,SLOT(FileReceive()));
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),this,
            SLOT(displayError(QAbstractSocket::SocketError))); 
}

void QAppWindow::FileReceive()
{
    if(!FileConnect)
    {
        QString str = "TRANS";
        if(str == tcpClient->readAll())
        {
            FileConnect = true;
            char *FileMessage = "ACCEPT";
            switch (QMessageBox::information(this,tr("文件传送请求"), tr("用户")+tcplink->friendInfo.account+tr("向您传送文件？\n是否接收"), "接收(&A)", "拒绝(&C)", 0))
            {
            case 0:
                tcpClient->write(FileMessage);
                ReceiveDialog->show();
                break;
            case 1:
                FileMessage = "REFUSE";
                tcpClient->write(FileMessage);
                break;
            default:
                tcpClient->write(FileMessage);
                ReceiveDialog->show();
                break;
            }
        }
    }
    else
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
                    qDebug() << "Open File Error!";
                    return;
                }
            }
            else return;
        }
        if(ReceiveFile.FinishedBytes < ReceiveFile.WholeBytes)
        {
            ReceiveFile.FinishedBytes += tcpClient->bytesAvailable();
            ReceiveFile.Buffer = tcpClient->readAll();
            ReceiveFile.File->write(ReceiveFile.Buffer);
            ReceiveFile.Buffer.resize(0);
        }
        ServerProgressBar->setMaximum(ReceiveFile.WholeBytes);
        ServerProgressBar->setValue(ReceiveFile.FinishedBytes);
        if(ReceiveFile.FinishedBytes == ReceiveFile.WholeBytes)
        {
            tcpClient->close();
            ReceiveFile.File->close();
            OkPushButton->setText(tr("接收成功"));
        }
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

void QAppWindow::on_OkPushButton_clicked()
{
    ReceiveDialog->close();
}
