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
#include "friendinfo.h"

extern TCPLink *tcplink;           // tcplink 全局变量
//extern QSqlDatabase *db;            // db 数据库
QTreeWidgetItem* clicked_item;

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
    ui->chat->setMouseTracking(true);
    ui->chat->setStyleSheet("QPushButton{border-image: url(:/mainpicture/chat_normal.png);background-image: url(:/mainpicture/kb.png);}"
                            "QPushButton:hover{border-image: url(:/mainpicture/chat_hover.png);background-image: url(:/mainpicture/kb.png);}"
                            );
    ui->qunchat->setMouseTracking(true);
    ui->qunchat->setStyleSheet("QPushButton{border-image: url(:/mainpicture/qunchat_normal.png);background-image: url(:/mainpicture/kb.png);}"
                               "QPushButton:hover{border-image: url(:/mainpicture/qunchat_hover.png);background-image: url(:/mainpicture/kb.png);}"
                              );
    ui->addchat->setMouseTracking(true);
    ui->addchat->setStyleSheet("QPushButton{border-image: url(:/mainpicture/addchat_normal.png);background-image: url(:/mainpicture/kb.png);}"
                               "QPushButton:hover{border-image: url(:/mainpicture/addchat_hover.png);background-image: url(:/mainpicture/kb.png);}"
                              );

    //设置加好友输入框不显示
    flag = false;
    ui->Edit_FriendAccount->setVisible(flag);
    ui->Button_queryFriend->setVisible(flag);
    ui->startadd->setVisible(false);
    ui->chatlabel->setVisible(false);
    ui->qunchatlabel->setVisible(false);
    ui->addchatlabel->setVisible(false);
    ui->addtreeWidget->setVisible(false);
    ui->quntreeWidget->setVisible(false);

    tcplink->initasServer();
    // 连接好友状态改变信号与好友列表刷新槽
    connect(tcplink, SIGNAL(friendstatusChangedSignal()), this, SLOT(refresh()));
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
    tcplink->databasetoFriendVect();
    for(int i = 0; i < tcplink->friendVect.size(); i++)
    {
        qDebug() << tcplink->friendVect[i];
    }

    //建treewidget
    build_tree();
    clicked_item = ui->treeWidget->topLevelItem(0);

    //treewidget边框隐藏
    ui->treeWidget->setFrameStyle(QFrame::NoFrame);
    ui->addtreeWidget->setFrameStyle(QFrame::NoFrame);
    ui->quntreeWidget->setFrameStyle(QFrame::NoFrame);

    //双击点击treewidget节点
    connect(ui->treeWidget,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(showSelectedImage(QTreeWidgetItem*,int)));
    //connect(ui->treeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(ItemClicked(QTreeWidgetItem*,int)));

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
    timer->start(500);

    MainIcon = new QSystemTrayIcon(this);
    connect(MainIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,
            SLOT(MainIconClicked(QSystemTrayIcon::ActivationReason)));
    QIcon icon(":/mainpicture/Mushroomonline.ico");
    MainIcon->setIcon(icon);
    MainIcon->show();
    MainIcon->showMessage(QString(tr("提示")),QString(tr("登录成功")));
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
// 查找所有的聊天窗口，如果找到相一致的聊天窗口返回相应的窗口序号，否则返回 -1 （注意：friendNo 不一定是好友编号的序号不一定一致）
int QAppWindow::findChatWindow(QVector<int> friendNo)
{
    for(int i = 0; i < chatVect.size(); i++)
    {
        if(friendNoEqual(chatVect[i]->friendNo, friendNo))
            return i;
    }
    return -1;  // 没有该窗口
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
    friendinfo *info;
    int friendNumber;
    QStringList accountList;
    QVector<int> friendNo;
    int chatNumber;

    switch (replyKind) {
    /** 查找好友结果 */
    case FRIEND_ONLINE:
        qDebug() << "已找到用户" << tcplink->friendInfo.account << "IP地址: " << tcplink->friendInfo.node.hostAddr;
//        tcplink->friendVect.
        // 还不是好友，请求加为好友
        if(-1 == tcplink->findAccount(tcplink->friendInfo.account))
        {
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
        }
        // 向服务器查找自身的 IP 地址
        else if(0 == tcplink->findAccount(tcplink->friendInfo.account))
        {
            QMessageBox::information(this, "笨笨哒", "你还不知道自己的IP吗~\n哈哈，我来告诉你吧："+tcplink->friendInfo.node.hostAddr+"\n记好咯~");
        }
        else    // 已经是好友了
        {
            switch(friendMsgBox.question( this, "你的好友",
                                          "好友在线\n"
                                          "现在开始聊天~",
                                          "开始聊天(&S)", "取消(&C)",
                                          0     /* Enter == button 0*/
                                          ))
            { // Escape == button 1
            case 0: // “开始聊天”或者 ALT + S 按下被
                // 发出开始聊天的请求
                friendMsgBox.setWindowTitle("loading。。。");
                friendMsgBox.setText("正在发出好友聊天请求\n"
                                     "等待好友确认聊天。。。");
//                tcplink->addFriendRequest();
                tcplink->startChatRequest();
                //tcplink->friendInfo.tcpSocket = tcpSender;

    //            chat = new chatWindow;
    //            chat->show();
                friendMsgBox.close();

                break;
            case 1: // Cancel被点击或者Alt+C被按下或者Escape被按下。
                // 不退出
                break;
            }
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
    /** 好友连接请求 */
    case CONNECT_REQUEST:

        break;
    case CONNECT_SUCCESS:

        break;
    /** 群聊请求 */
    case GROUPCHAT_REQUEST:
        accountList = tcplink->groupString.split("_");      // 按 "_" 截取好友
//        friendInfo.account = accountList[0];         // 发起请求的是第一个好友
        friendNo.clear();
        for(int i = 0; i < accountList.size(); i++)
        {
            friendNo.push_back(tcplink->findAccount(accountList[i]));
        }
        // 打开群聊窗口
        chatNumber = findChatWindow(friendNo);
        if(-1 == chatNumber)    // 没有该窗口
        {
            chat = new chatWindow(friendNo, false);       // 成功添加好友，打开聊天窗口，不是群聊的发起者
            // 设置单人聊天窗口为相应的头像和账号，可在 chatWindow 中实现
            // TODO：头像
            chat->setWindowTitle(tcplink->friendInfo.account);   // 账号
            connect(this,SIGNAL(FriendInfoSignal(FriendInfo)),chat,SLOT(GetFriendInfo(FriendInfo)));
            chat->show();
            emit this->FriendInfoSignal(tcplink->friendInfo);
            chatVect.push_back(chat);
//            replyKind = ADDFRIEND_SUCCESS;   // 成功添加好友
        }
        else
        {
            chatVect[chatNumber]->initSocket();  // 重新检查在线状态
            chatVect[chatNumber]->show();    // 有的话直接打开该窗口继续聊天
        }
        ///
        ///
        ///
        ///
        break;

    /** 好友请求 */
    case ADDFRIEND_REQUEST:
        switch (QMessageBox::information(this,tr("好友请求"), tr("用户")+tcplink->friendInfo.account+tr("请求加为好友？\n是否加为好友"), "加为好友(&A)", "取消(&C)", 0)) {
        case 0:  // 加为好友 - 默认
            tcplink->friendInfo.tcpSocket->write(tr("ADD").toStdString().c_str());       // 发送接收好友请求

            // 成功添加好友，打开完善好友信息窗口
            info = new friendinfo(NULL);
            connect(info, SIGNAL(addfriendinfoSignal()), this, SLOT(addfriendinfo()));      // 连接成功添加好友信号槽
            info->show();

//            if(-1 == tcplink->findAccount(tcplink->friendInfo.account))
//            {
//                tcplink->friendVect.push_back(tcplink->friendInfo);  // 添加好友
//                tcplink->databaseInsert(tcplink->friendInfo);   // 同时更新好友数据库
//            }
// //           friendInfo = new
//            /** 显示在主界面窗口上 */
//            friendNumber = tcplink->findAccount(tcplink->friendInfo.account);
//            friendNo.clear();
//            friendNo.push_back(friendNumber);
//            tcplink->friendVect[friendNumber].tcpSocket = tcplink->friendInfo.tcpSocket;
//            tcplink->friendVect[friendNumber].isConnected = tcplink->friendInfo.isConnected;
//            tcplink->friendVect[friendNumber].node.hostAddr = tcplink->friendInfo.node.hostAddr;

//            chat = new chatWindow(friendNo);       // 成功添加好友，打开聊天窗口
//            // 设置单人聊天窗口为相应的头像和账号，可在 chatWindow 中实现
//            // TODO：头像
//            chat->setWindowTitle(tcplink->friendInfo.account);   // 账号
//            connect(this,SIGNAL(FriendInfoSignal(FriendInfo)),chat,SLOT(GetFriendInfo(FriendInfo)));
//            chat->show();
//            emit this->FriendInfoSignal(tcplink->friendInfo);
////            replyKind = ADDFRIEND_SUCCESS;   // 成功添加好友
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
//        tcplink->friendVect.push_back(tcplink->friendInfo);
//        tcplink->databaseInsert(tcplink->friendInfo);
        // 成功添加好友，打开完善好友信息窗口
        info = new friendinfo(NULL);
        connect(info, SIGNAL(addfriendinfoSignal()), this, SLOT(addfriendinfo()));      // 连接成功添加好友信号槽
        info->show();
//        if(-1 == tcplink->findAccount(tcplink->friendInfo.account))
//        {
//            tcplink->friendVect.push_back(tcplink->friendInfo);  // 添加好友
//            tcplink->databaseInsert(tcplink->friendInfo);   // 同时更新好友数据库
//        }
////           friendInfo = new
//        /** 显示在主界面窗口上 */
//        friendNumber = tcplink->findAccount(tcplink->friendInfo.account);
//        friendNo.clear();
//        friendNo.push_back(friendNumber);
//        tcplink->friendVect[friendNumber].tcpSocket = tcplink->friendInfo.tcpSocket;
//        tcplink->friendVect[friendNumber].isConnected = tcplink->friendInfo.isConnected;
//        tcplink->friendVect[friendNumber].node.hostAddr = tcplink->friendInfo.node.hostAddr;

//        chat =new chatWindow(friendNo);
//        chat->setWindowTitle(tcplink->friendInfo.account);
//        connect(this,SIGNAL(FriendInfoSignal(FriendInfo)),chat,SLOT(GetFriendInfo(FriendInfo)));
//        chat->show();
//        emit this->FriendInfoSignal(tcplink->friendInfo);
        break;
    case ADDFRIEND_DENY:    // 好友拒绝添加好友请求
        qDebug() << "用户" << tcplink->friendInfo.account << "拒绝添加好友请求TAT";
        break;

    /** 聊天请求 */
    case STARTCHAT_REQUEST:
        switch (QMessageBox::information(this,tr("好友请求"), tr("好友")+tcplink->friendInfo.account+tr("发起聊天？\n是否接受聊天请求"), "开始聊天(&A)", "取消(&C)", 0)) {
        case 0:  // 加为好友 - 默认
            tcplink->friendInfo.tcpSocket->write(tr("start_chat").toStdString().c_str());       // 发送接收好友请求
            qDebug() << "start_chat";
//            tcplink->friendVect.push_back(tcplink->friendInfo);  // 添加好友
//            tcplink->databaseInsert(tcplink->friendInfo);   // 同时更新好友数据库
 //           friendInfo = new
            /** 显示在主界面窗口上 */
            friendNo.clear();
            friendNo.push_back(tcplink->findAccount(tcplink->friendInfo.account));
            chatNumber = findChatWindow(friendNo);
            if(-1 == chatNumber)    // 没有该窗口
            {
                chat = new chatWindow(friendNo);       // 成功添加好友，打开聊天窗口
                // 设置单人聊天窗口为相应的头像和账号，可在 chatWindow 中实现
                // TODO：头像
                chat->setWindowTitle(tcplink->friendInfo.account);   // 账号
                connect(this,SIGNAL(FriendInfoSignal(FriendInfo)),chat,SLOT(GetFriendInfo(FriendInfo)));
                chat->show();
                emit this->FriendInfoSignal(tcplink->friendInfo);
                chatVect.push_back(chat);
    //            replyKind = ADDFRIEND_SUCCESS;   // 成功添加好友
            }
            else
            {
                chatVect[chatNumber]->initSocket();  // 重新检查在线状态
                chatVect[chatNumber]->show();    // 有的话直接打开该窗口继续聊天
            }


            break;
        case 1:
            tcplink->friendInfo.tcpSocket->write(tr("cancel_chat").toStdString().c_str());    // 发送取消好友请求
 //           replyKind = ADDFRIEND_DENY;      // 好友拒绝
            break;
        default:
            break;
        }
        break;
    case STARTCHAT_SUCCESS: // 成功发起聊天
//        tcplink->friendVect.push_back(tcplink->friendInfo);
//        tcplink->databaseInsert(tcplink->friendInfo);
        /** 显示在主界面窗口上 */
        friendNo.clear();
        friendNo.push_back(tcplink->findAccount(tcplink->friendInfo.account));
        chatNumber = findChatWindow(friendNo);
        if(-1 == chatNumber)    // 没有该窗口
        {
            chat = new chatWindow(friendNo);       // 成功添加好友，打开聊天窗口
            // 设置单人聊天窗口为相应的头像和账号，可在 chatWindow 中实现
            // TODO：头像
            chat->setWindowTitle(tcplink->friendInfo.account);   // 账号
            connect(this,SIGNAL(FriendInfoSignal(FriendInfo)),chat,SLOT(GetFriendInfo(FriendInfo)));
            chat->show();
            emit this->FriendInfoSignal(tcplink->friendInfo);
            chatVect.push_back(chat);
//            replyKind = ADDFRIEND_SUCCESS;   // 成功添加好友
        }
        else
        {
            chatVect[chatNumber]->initSocket();  // 重新检查在线状态
            chatVect[chatNumber]->show();    // 有的话直接打开该窗口继续聊天
        }
        tcplink->replyKind = 0;
        break;
    case STARTCHAT_DENY:    // 好友拒绝聊天请求
        qDebug() << "好友" << tcplink->friendInfo.account << "拒绝聊天请求TAT\n还能不能一起愉快的玩耍了";
        break;
    default:
        break;
    }
}
// 处理成功添加好友槽
void QAppWindow::addfriendinfo()
{
    QAppWindow::build_tree();   // 更新好友列表
    QAppWindow::refresh();      // 刷新好友列表状态
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
//            tcpClient->peerAddress().toString();并相应的查找 friendVect[] 中的相关信息
            switch (QMessageBox::information(this,tr("文件传送请求"), tr("用户")+tcplink->friendInfo.account+tr("向您传送文件？\n是否接收"), "接收(&A)", "拒绝(&C)", 0))   // TODO：找到谁发送的文件，而不一定是 tcplink->friendInfo.account 通过 tcpClient的IP地址
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
//    this->hide();   // 隐藏而不是直接关闭
    this->close();
}

void QAppWindow::on_qappminButton_clicked()
{
    //this->showMinimized();
    this->hide();
}

void QAppWindow::on_addfriend_clicked()
{
    flag = !flag;
    ui->Edit_FriendAccount->setVisible(flag);
    ui->Button_queryFriend->setVisible(flag);

}

//-----------------------------treewidget---------------------------------------

//建TreeWidget
void QAppWindow::build_tree()
{
    ui->treeWidget->setStyleSheet("QTreeWidget::item{height:40px}");
    ui->treeWidget->setIconSize(QSize(40,40));

    QSqlQuery p;
    p.exec("select * from groups");      // 指定查找数据库中的groups表

    QTreeWidgetItem *group[100]; //最多设10个分组,每组最多10个人
    int i = 0;

    while(p.next())
    {
        QString groupname = p.value(1).toString(); //读组名
        QString member = p.value(2).toString(); //读成员学号
        //qDebug()<<groupname<<member;

        group[i] = new QTreeWidgetItem(ui->treeWidget,QStringList(groupname));//建组名的父节点
        group[i]->setData(0,Qt::UserRole,groupname);

        int j = 0;
        QSqlQuery q;
        q.exec("select * from friends");      // 指定查找数据库中的friends表
        while(q.next())
        {
            QString ip = q.value(1).toString();
            if((member.indexOf(ip)>=0)&&(member.indexOf(ip)<=member.size()))
            {
                //qDebug() << ip << member.indexOf(ip);
                //建好友子节点
                group[10*(i+1)+j] = new QTreeWidgetItem(group[i],QStringList(q.value(2).toString()));
                group[10*(i+1)+j]->setIcon(0,QIcon(q.value(3).toString()));
                group[10*(i+1)+j]->setData(0,Qt::UserRole,(q.value(1).toString()));
//                new_treewidgetitem *_item = new new_treewidgetitem;
//                group[10*(i+1)+j] = new QTreeWidgetItem;
//                group[10*(i+1)+j]->addChild(group[i]);
//                ui->treeWidget->setItemWidget(group[10*(i+1)+j],0,_item);
            }
            j++;
        }
        i++;
    }
//    QTreeWidgetItem *group = new QTreeWidgetItem(ui->treeWidget,QStringList(QString("clssmates")));
//    QTreeWidgetItem *friend1 = new QTreeWidgetItem(group,QStringList(QString(" weijie")));
//    friend1->setIcon(0,QIcon(":/picture/tx1.jpg"));
    ui->treeWidget->expandAll(); //结点全部展开
    return;
    //db.close();
}


//treewidget节点双击响应（聊天）
void QAppWindow::showSelectedImage(QTreeWidgetItem *item, int column)
{
    //qDebug()<<"haha";
    QTreeWidgetItem *parent = item->parent();
    if(NULL==parent) //注意：最顶端项是没有父节点的，双击这些项时注意(陷阱)
         return;
    QVariant var = item->data(0,Qt::UserRole);
    QString student_ID = var.toString();//读出好友的学号信息

    //开始聊天
    // TODO: 重写开始聊天
    tcplink->friendInfo.account = student_ID;
//    tcplink->friendInfo = tcplink->friendVect[tcplink->findAccount(student_ID)];
    // 确定好友在线，发出聊天请求
    tcplink->friendInfo.node.hostAddr = tcplink->friendVect[tcplink->findAccount(student_ID)].node.hostAddr;
    if(ONLINE == tcplink->confirmFriendOnline(student_ID))
    {
         tcplink->startChatRequest();    // 发出聊天请求
         qDebug() << "start chat with " << student_ID;
    }
    ///
    ///
    ///
    ///
    ///

//    chatWindow *chat;
//    chat = new chatWindow;       // 打开聊天窗口 学号为student_ID
//    chat->show();

}


//添加treewidget节点右键菜单用于删除好友
void QAppWindow::on_treeWidget_customContextMenuRequested(const QPoint &pos)
{
    curItem = ui->treeWidget->itemAt(pos);  //获取当前被点击的节点
    if(curItem==NULL)return;           //这种情况是右键的位置不在treeItem的范围内，即在空白位置右击
    QVariant var = curItem->data(0,Qt::UserRole);//读出该好友的学号

    if(curItem->parent() == NULL)   //分组的右键菜单
    {
       QMenu *popMenu =new QMenu(this);//定义一个右键弹出菜单
       QAction *delegroup = popMenu->addAction("Delete this Group");
       connect(delegroup,SIGNAL(triggered(bool)), this, SLOT(onDeleGroup()));
       popMenu->exec(QCursor::pos());//弹出右键菜单，菜单位置为光标位置
    }
    else //好友的右键菜单
    {
        QMenu *popMenu =new QMenu(this);//定义一个右键弹出菜单
        QAction *delefriend = popMenu->addAction("Delete this Friend");
        connect(delefriend,SIGNAL(triggered(bool)), this, SLOT(onDeleFriend()));
        popMenu->exec(QCursor::pos());//弹出右键菜单，菜单位置为光标位置
    }
}

void QAppWindow::onDeleGroup()
{
    QVariant var = curItem->data(0,Qt::UserRole);
    if(var.toString()=="默认分组")
    {
        QMessageBox::information(this, tr("info"), tr("默认分组不可删除"), QMessageBox::Ok);
    }
    else
    {
        //QTreeWidgetItem* m_currentItem = ui->treeWidget->itemAt(pos);
        int i = ui->treeWidget->indexOfTopLevelItem(curItem);
        curItem->takeChildren();
        ui->treeWidget->takeTopLevelItem(i);

        QString s = "delete from groups where name='"+var.toString()+"'";
        //qDebug()<<s;
        QSqlQuery p;
        p.exec(s);
    }

    //插入语句：insert into 表名(字段列表) values(值列表)。如： insert into person(name, age) values(‘传智’,3)
    //更新语句：update 表名 set 字段名=值 where 条件子句。如：update person set name=‘传智‘ where id=10
    //delete from 表名 where 条件子句。如：delete from person  where id=10

}

void QAppWindow::onDeleFriend()
{
    //QTreeWidgetItem* m_currentItem = ui->treeWidget->itemAt(pos);
    curItem->parent()->removeChild(curItem);
    QVariant var = curItem->data(0,Qt::UserRole);
    QString s = "delete from friends where account='"+var.toString()+"'";
    //qDebug()<<s;
    QSqlQuery p;
    p.exec(s);
}


//在treewidget里点击鼠标刷新好友列表
void QAppWindow::refresh()
{
    for(int i = 0; i< ui->treeWidget->topLevelItemCount();i++)
    {
        QTreeWidgetItem *topitem = ui->treeWidget->topLevelItem(i);
        for(int j = 0; j< topitem->childCount();j++)
        {
            QTreeWidgetItem *frienditem= topitem->child(j);
            QVariant sid_v = frienditem->data(0,Qt::UserRole);
            QString sid_s = sid_v.toString();
            QSqlQuery fri;
            fri.exec("select * from friends");      // 指定查找数据库中的friends表
            while(fri.next())
            {
                if(fri.value(1).toString() == sid_s)
                {
                    if(fri.value(5).toInt()==1)
                        frienditem->setIcon(0,QIcon(fri.value(3).toString()));//在线头像
                    if(fri.value(5).toInt()==0)
                        frienditem->setIcon(0,QIcon(fri.value(8).toString()));//不在线头像
                }
            }
        }
    }

//    while (ui->treeWidget->topLevelItemCount() > 0 )
//    {
//        //qDebug("%d",ui->treeWidget->topLevelItemCount());
//        QTreeWidgetItem *parent = ui->treeWidget->takeTopLevelItem(0);
//        parent->takeChildren ();
//        //释放掉存放节点的内存空间
//        int childCount=parent->childCount();//子节点数
//        for (int i=0;i<childCount;i++)
//        {
//            QTreeWidgetItem* item = parent->child(0);
//            delete item;
//            item=NULL;
//        }
//        delete parent;
//        parent=NULL;
//    }
//    build_tree();
}

void QAppWindow::on_OkPushButton_clicked()
{
    ReceiveDialog->close();
}


//点击“我的好友”
void QAppWindow::on_chat_clicked()
{
    ui->startadd->setVisible(false);
    ui->chatlabel->setVisible(true);
    ui->qunchatlabel->setVisible(false);
    ui->addchatlabel->setVisible(false);
    ui->treeWidget->setVisible(true);
    ui->addtreeWidget->setVisible(false);
    ui->quntreeWidget->setVisible(false);

    while (ui->treeWidget->topLevelItemCount() > 0 )
    {
       //qDebug("%d",ui->treeWidget->topLevelItemCount());
       QTreeWidgetItem *parent = ui->treeWidget->takeTopLevelItem(0);
       parent->takeChildren ();
       //释放掉存放节点的内存空间
       int childCount=parent->childCount();//子节点数
       for (int i=0;i<childCount;i++)
       {
           QTreeWidgetItem* item = parent->child(0);
           delete item;
           item=NULL;
       }
       delete parent;
       parent=NULL;
    }
    build_tree();
}


//点击“我的群聊”
void QAppWindow::on_qunchat_clicked()
{
    ui->startadd->setVisible(false);
    ui->chatlabel->setVisible(false);
    ui->qunchatlabel->setVisible(true);
    ui->addchatlabel->setVisible(false);
    ui->treeWidget->setVisible(false);
    ui->addtreeWidget->setVisible(false);
    ui->quntreeWidget->setVisible(true);

    ui->quntreeWidget->setStyleSheet("QTreeWidget::item{height:40px}");
    ui->quntreeWidget->setIconSize(QSize(40,40));

    //清除原节点
    while (ui->quntreeWidget->topLevelItemCount() > 0 )
    {
       //qDebug("%d",ui->treeWidget->topLevelItemCount());
       QTreeWidgetItem *item = ui->quntreeWidget->takeTopLevelItem(0);
       delete item;
       item=NULL;
    }

    QSqlQuery p;
    p.exec("select * from blocks");
    QTreeWidgetItem * qunchat[50];
    int i = 0;
    while(p.next())
    {
        QString member = p.value(2).toString(); //读群聊名
        qunchat[i] = new QTreeWidgetItem(ui->quntreeWidget,QStringList(member));//建组名的父节点
        qunchat[i]->setIcon(0,QIcon(":/mainpicture/group.ico"));
        i++;
    }
    ui->quntreeWidget->expandAll(); //结点全部展开
}



//点击“添加群聊”
void QAppWindow::on_addchat_clicked()
{
    ui->startadd->setVisible(true);
    ui->chatlabel->setVisible(false);
    ui->qunchatlabel->setVisible(false);
    ui->addchatlabel->setVisible(true);
    ui->treeWidget->setVisible(false);
    ui->addtreeWidget->setVisible(true);
    ui->quntreeWidget->setVisible(false);

    ui->addtreeWidget->setStyleSheet("QTreeWidget::item{height:40px}");
    ui->addtreeWidget->setIconSize(QSize(40,40));

    //清除原节点
    while (ui->addtreeWidget->topLevelItemCount() > 0 )
    {
       //qDebug("%d",ui->treeWidget->topLevelItemCount());
       QTreeWidgetItem *item = ui->addtreeWidget->takeTopLevelItem(0);
       delete item;
       item=NULL;
    }

    QSqlQuery p;
    p.exec("select * from friends");      // 指定查找数据库中的groups表
    QTreeWidgetItem * addchat[50];
    int i = 0;
    while(p.next())
    {
        // 注意：群聊窗口里不要显示自己啊
        if(p.value(1).toString() == tcplink->loginInfo.account) // 群聊窗口不显示自己
            continue;
        QString name = p.value(2).toString(); //读名字
        QString member = p.value(1).toString(); //读学号
        //qDebug()<<groupname<<member;

        addchat[i] = new QTreeWidgetItem(ui->addtreeWidget,QStringList(name));//建组名的父节点
        addchat[i]->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |Qt::ItemIsEnabled);
        addchat[i]->setCheckState(0, Qt::Unchecked);
        addchat[i]->setData(0,Qt::UserRole,member);
        addchat[i]->setIcon(0,QIcon(p.value(3).toString()));
        i++;
    }
    ui->addtreeWidget->expandAll(); //结点全部展开
}


//开始群聊
void QAppWindow::on_startadd_clicked()
{
    QString s;
    QVector<int> friendNo;
    QString selected;
    int chatNumber;
    for(int i = 0; i<ui->addtreeWidget->topLevelItemCount();i++)
    {
        QTreeWidgetItem* item = ui->addtreeWidget->topLevelItem(i);
        if(item->checkState(0)== Qt::Checked)
        {
            selected = (item->data(0,Qt::UserRole)).toString();
            s = s + selected + ",";
            friendNo.append(tcplink->findAccount(selected));
        }
    }
    qDebug()<<s;
    if(friendNo.size() == 0)    // 一个也没选，不开启群聊
        return ;
    QSqlQuery p;
    // 检查是否已经存在这组群聊信息
    p.exec("insert into blocks(members) values('"+s+"')");      // 注意同时更新群聊窗口（是否已经存在该群聊）

    //打开群聊窗口chatwindow
    tcplink->startGroupChat(friendNo);
    // 打开群聊窗口
    chatNumber = findChatWindow(friendNo);
    if(-1 == chatNumber)    // 没有该窗口
    {
        chatWindow *chat;
        chat = new chatWindow(friendNo, true);       // 发起群聊，打开聊天窗口，是群聊的发起者
        // 设置单人聊天窗口为相应的头像和账号，可在 chatWindow 中实现
        // TODO：头像
        chat->setWindowTitle(tcplink->friendInfo.account);   // 账号
        connect(this,SIGNAL(FriendInfoSignal(FriendInfo)),chat,SLOT(GetFriendInfo(FriendInfo)));
        chat->show();
        emit this->FriendInfoSignal(tcplink->friendInfo);
        chatVect.push_back(chat);
//            replyKind = ADDFRIEND_SUCCESS;   // 成功添加好友
    }
    else
    {
        chatVect[chatNumber]->initSocket();  // 重新检查在线状态
        chatVect[chatNumber]->beStarter = false;    // 需要重新检查在线状态
        chatVect[chatNumber]->show();    // 有的话直接打开该窗口继续聊天
    }



}

void QAppWindow::MainIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    //单击
    case QSystemTrayIcon::Trigger:
    //双击
    case QSystemTrayIcon::DoubleClick:
        if(this->isHidden())
        {
            //恢复窗口显示
            this->show();

            this->setWindowState(Qt::WindowActive);
            this->activateWindow();
        }
        else
        {
            this->hide();
        }
        break;
    default:
        break;
    }
}
