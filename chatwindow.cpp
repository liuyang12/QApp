#include "chatwindow.h"
#include "ui_chatwindow.h"
#include <QDateTime>
#include "tcplink.h"
#include <algorithm>

extern TCPLink *tcplink;           // tcplink 全局变量

chatWindow::chatWindow(QVector<int> frNo, QWidget *parent):
    QDialog(parent),
    friendNo(frNo),     // 传递参与聊天的所有好友的序号列表
    ui(new Ui::chatWindow)
{
    ui->setupUi(this);
    //
//    this->setWindowTitle();
    tcpClient = new QTcpSocket(this);
    FileConnect = false;
    //建立发送文件的Socket
    ui->sendFileButton->setEnabled(false);
    //初始化发送文件不可用，需先打开文件
    ui->clientStatusLabel->setVisible(false);
    ui->clientProgressBar->setVisible(false);
    connect(tcpClient,SIGNAL(readyRead()),this,SLOT(StartTransmit()));
    //连接服务器成功，发出connected()信号，开始传送文件
    connect(tcpClient,SIGNAL(bytesWritten(qint64)),this,SLOT(UpdateProgressBar(qint64)));
    //更新进度条
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),this,
            SLOT(displayError(QAbstractSocket::SocketError)));

    // 聊天窗口初始化
    blockSize = 0;
    ui->Show_message->setReadOnly(true);    // 消息显示窗口只读
    ui->Edit_message->installEventFilter(this);

    // 窗口的初始化
    this->initSocket();     // TCPSocket 通信初始化
    this->initWindowHead(); // 窗口标题和头像初始化
}

chatWindow::~chatWindow()
{
    tcpClient->close();
    delete ui;
}
// 重写事件过滤虚函数
bool chatWindow::eventFilter(QObject *obj, QEvent *ev)
{
    Q_ASSERT(obj == ui->Edit_message);  // 目标设定为 Edit_message，使之响应 按键
    if(ev->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyevent = static_cast<QKeyEvent *>(ev);
        if(keyevent->key() == Qt::Key_Return/* && (keyevent->modifiers() & Qt::ControlModifier)*/) // 回车键按下即发送消息
        {
            chatWindow::on_sendMsgButton_clicked(); // 发送消息
            return true;
        }
    }
    return false;
}
bool friendNoEqual(QVector<int> friendNo_a, QVector<int> friendNo_b)
{
    if(friendNo_a.size() != friendNo_b.size())
        return false;   // 个数不相等则为不等
    std::sort(friendNo_a.begin(), friendNo_a.end());    // 分别对两个 vector 进行排序
    std::sort(friendNo_b.begin(), friendNo_b.end());
    return (friendNo_a == friendNo_b);  // 重新排序后比较大小
}

// 重载等号，如果两个窗口 friendNo 一致（不考虑序号），则两个窗口相等
bool chatWindow::operator ==(const chatWindow &chat)
{
    return (friendNoEqual(friendNo, chat.friendNo));
}

// 初始化 TCPSocket 通信
void chatWindow::initSocket()
{
    FriendInfo tmpfriend;   // 临时转存
    tmpfriend = tcplink->friendInfo;    // 首先保存临时好友信息
    HeadString = "";
    qDebug() << friendNo;
    for(int i = 0; i < friendNo.size(); i++)
    {   HeadString = HeadString + tcplink->friendVect[friendNo[i]].account + " ";
        // 每次打开聊天窗口确定所有好友的在线状态
        if(ONLINE == tcplink->confirmFriendOnline(tcplink->friendVect[friendNo[i]].account) || IPUPDATED == tcplink->confirmFriendOnline(tcplink->friendVect[friendNo[i]].account))  // 在线
        {
            if(!tcplink->friendVect[friendNo[i]].isConnected)   // 如果还未连接则进行连接
            {
                tcplink->friendInfo = tcplink->friendVect[friendNo[i]];
                tcplink->connectRequest();
            }
//            tcplink->requestKind = CONNECT;
//            tcplink->newTCPConnection();    // 获取新的 TCPSocket 用于通信
            // 每次使用 tcpSocket 需要确保其在线
            // 连接所有在线好友的 TCPSocket
            connect(tcplink->friendVect[friendNo[i]].tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));        // 只要有可读消息，则读取所有好友的消息
            connect(tcplink->friendVect[friendNo[i]].tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displaySocketError(QAbstractSocket::SocketError)));
        }
    }
    tcplink->friendInfo = tmpfriend;    // 恢复临时好友信息

////    connect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendMessage()));
//    connect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));      // 读取好友消息
//    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displaySocketError(QAbstractSocket::SocketError)));   // 显示错误信息
}
// 设置窗口标题和头像
void chatWindow::initWindowHead()
{
    // 设置窗口标题
    this->setWindowTitle(HeadString);
    // 设置窗口头像
    // TODO
}

void chatWindow::GetFriendInfo(FriendInfo info)
{
    friendInfo = info;
    tcplink->disconnectfriendSocket();
    friendInfo.tcpSocket = info.tcpSocket;
//    initSocket();
}

//打开文件
void chatWindow::on_openFileButton_clicked()
{
    SendFile.EachSize = 4*1024;
    SendFile.FileName = QFileDialog::getOpenFileName(this);
    if(!SendFile.FileName.isEmpty())
    {
        ui->sendFileButton->setEnabled(true);
        ui->clientStatusLabel->setVisible(true);
        ui->clientStatusLabel->setText(tr("打开成功"));
    }
}

//连接并发送文件
void chatWindow::on_sendFileButton_clicked()
{
    ui->sendFileButton->setEnabled(false);
    ui->clientProgressBar->setVisible(true);
    SendFile.FinishedBytes = 0;
    FileConnect = false;
    ui->clientStatusLabel->setText(tr("正在连接"));
    tcpClient->connectToHost(friendInfo.node.hostAddr, /*getPortNumber(friendInfo.account)*/16666);
    char *Mess = "TRANS";
    tcpClient->write(Mess);
}

void chatWindow::StartTransmit()
{
    if(!FileConnect)
    {
        QString str = "ACCEPT";
        if(str == tcpClient->readAll())
        {
            FileConnect = true;
            SendFile.File = new QFile(SendFile.FileName);
            if(!SendFile.File->open(QFile::ReadOnly))
            {
                qDebug() << "open file error!";
                return;
            }
            SendFile.WholeBytes = SendFile.File->size();
            QDataStream Out(&SendFile.Buffer,QIODevice::WriteOnly);
            Out.setVersion(QDataStream::Qt_5_3);
            QString current = SendFile.FileName.right(SendFile.FileName.size() - SendFile.FileName.lastIndexOf('/')-1);
            Out << qint64(0) << qint64(0) << current;
            SendFile.WholeBytes += SendFile.Buffer.size();
            Out.device()->seek(0);
            Out << SendFile.WholeBytes << qint64((SendFile.Buffer.size() - sizeof(qint64)*2));
            SendFile.TodoBytes = SendFile.WholeBytes - tcpClient->write(SendFile.Buffer);
            ui->clientStatusLabel->setText(tr("开始传输"));
            SendFile.Buffer.resize(0);
        }
        else
            ui->clientStatusLabel->setText(tr("对方拒收"));
    }
}

void chatWindow::UpdateProgressBar(qint64 temp)
{
    if(FileConnect)
    {
        ui->clientStatusLabel->setText(tr("正在传输"));
        SendFile.FinishedBytes += (int)temp;
        if(SendFile.TodoBytes > 0)
        {
            SendFile.Buffer = SendFile.File->read(qMin(SendFile.TodoBytes,SendFile.EachSize));
            SendFile.TodoBytes -= (int)tcpClient->write(SendFile.Buffer);
            SendFile.Buffer.resize(0);
        }
        else
        {
            SendFile.File->close();
        }
        ui->clientProgressBar->setMaximum(SendFile.WholeBytes);
        ui->clientProgressBar->setValue(SendFile.FinishedBytes);

        if(SendFile.FinishedBytes == SendFile.WholeBytes)
        {
            ui->clientStatusLabel->setText(tr("发送成功").arg(SendFile.FileName));
            SendFile.File->close();
            tcpClient->close();
        }
    }
}

void chatWindow::displayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpClient->errorString();
    tcpClient->close();
    ui->clientProgressBar->reset();
    ui->clientStatusLabel->setText(tr("传输异常"));
    ui->sendFileButton->setEnabled(true);
}
// 连接成功，发送消息
void chatWindow::sendMessage()
{
    // 发送消息
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    out << (qint16)0;   // 预存数据大小
    out << sendString;        // 发送消息
    out.device()->seek(0);  // 回到首部
    out << (qint16)(block.size() - sizeof(qint16));
    for(int i = 0; i < friendNo.size(); i++)    // 向所有在线好友发送窗口数据
    {
        // 每次使用 tcpSocket 需要确保在线
        if(ONLINE == tcplink->friendVect[friendNo[i]].status)
            tcplink->friendVect[friendNo[i]].tcpSocket->write(block);
    }
//    friendInfo.tcpSocket->write(block); // 发送数据
}
// 更新显示窗口
void chatWindow::appendShowLine(QString &account)
{
    // 将接收到的信息显示在输出框
    QString datetime = getCurrentDateTime();
    /// todo 最前面加好友昵称
    QString temp = QString("<font size=\"3\" color=blue>%1  (<font color=dodgerblue><u>%2</u></font>) %3</font>%4").arg(tcplink->friendVect[tcplink->findAccount(account)].name).arg(account).arg(datetime).arg(recieveString);
    ui->Show_message->append(temp); // 显示在输出框
}

// ，准备接收，接收消息
void chatWindow::readMessage()
{
    for(int i = 0; i < friendNo.size(); i++)
    {
        if(ONLINE == tcplink->friendVect[friendNo[i]].status)
        {
            // 注意对于没有发送消息的好友的处理
//            if(tcplink->friendVect[friendNo[i]].tcpSocket->bytesAvailable())
            QDataStream in(tcplink->friendVect[friendNo[i]].tcpSocket);
            in.setVersion(QDataStream::Qt_5_3);
            if(blockSize == 0)
            {
                if(tcplink->friendVect[friendNo[i]].tcpSocket->bytesAvailable() < (int)sizeof(qint16))  // 自动对没有消息的好友进行了处理，直接返回，没有任何操作
                {
                    continue ;  // 此处是不是应该记录每个消息的归属
                }
                in >> blockSize;
            }
            if(tcplink->friendVect[friendNo[i]].tcpSocket->bytesAvailable() < blockSize)
                return ;
            in >> recieveString;
            if (recieveString == "")
                return ;
            qDebug() << recieveString;
            blockSize = 0;  // 重新归零
            // 将接收到的消息显示在输出框
            chatWindow::appendShowLine(tcplink->friendVect[friendNo[i]].account);
        }
    }
}



// 显示错误提示信息
void chatWindow::displaySocketError(QAbstractSocket::SocketError socketError)
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

// 获取当前日期时间
QString chatWindow::getCurrentDateTime()
{
     QDateTime datetime = QDateTime::currentDateTime();
     return datetime.toString("yyyy/M/d h:m:s");   // 获取当前时间 格式如 2015/1/1 6:36:11
//    QTime time = QTime::currentTime();
//    QDate date = QDate::currentDate();
//    return QString("%1 %2").arg(date.toString(Qt::ISODate)).arg(time.toString(Qt::ISODate));
}

// 发送消息按下
void chatWindow::on_sendMsgButton_clicked()
{
    // 如果输入框为空，则忽略此消息，不予发送
    if(ui->Edit_message->toPlainText().isEmpty())
        return ;
    // 获取输入框消息，并更新输出框
    QString tmpString = ui->Edit_message->toHtml();    // 以Html格式发送
    ui->Edit_message->clear();  // 输入框清空
    QString datetime = getCurrentDateTime();
    /// todo 最前面加自己的昵称
    sendString = QString("<font size=\"3\" color=green>%1  (<font color=dodgerblue><u>%2</u></font>) %3</font>%4").arg(tcplink->friendVect[0].name).arg(tcplink->loginInfo.account).arg(datetime).arg(tmpString);   // 转为 HTML 账号-时间-消息
    ui->Show_message->append(sendString);   // 显示在输入窗口
    // 发送消息
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    out << (qint16)0;   // 预存数据大小
    out << tmpString;        // 发送消息
    out.device()->seek(0);  // 回到首部
    out << (qint16)(block.size() - sizeof(qint16));
    for(int i = 0; i < friendNo.size(); i++)
    {
        // 每次访问 tcpSocket 确保用户在线并 Socket 已经创建
        if(ONLINE == (tcplink->friendVect[friendNo[i]].status))
            tcplink->friendVect[friendNo[i]].tcpSocket->write(block);       // 向每个在线好友发送消息
    }
//    friendInfo.tcpSocket->write(block); // 发送数据
    qDebug() << tmpString;

}

//void chatWindow::on_comboBox_activated(int index)
//{

//}
