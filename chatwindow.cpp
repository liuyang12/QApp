#include "chatwindow.h"
#include "ui_chatwindow.h"
#include "tcplink.h"

extern TCPLink *tcplink;           // tcplink 全局变量

chatWindow::chatWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::chatWindow)
{
    ui->setupUi(this);
    tcpClient = new QTcpSocket(this);
    ui->sendFileButton->setEnabled(false);

    connect(tcpClient,SIGNAL(connected()),this,SLOT(StartTransmit()));
    //当连接服务器成功时，发出connected()信号，我们开始传送文件
    connect(tcpClient,SIGNAL(bytesWritten(qint64)),this,SLOT(UpdateProgressBar(qint64)));
    //当有数据发送成功时，我们更新进度条
    connect(tcpClient,SIGNAL(error(QAbstractSocket::SocketError)),this,
            SLOT(displayError(QAbstractSocket::SocketError)));

    // 聊天窗口初始化
    blockSize = 0;
    ui->Show_message->setReadOnly(true);    // 消息显示窗口只读
}

chatWindow::~chatWindow()
{
    tcpClient->close();
    delete ui;
}

void chatWindow::initSocket()
{
//    connect(friendInfo.tcpSocket, SIGNAL(connected()), this, SLOT(sendMessage()));
    connect(friendInfo.tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage()));      // 读取好友消息
    connect(friendInfo.tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displaySocketError(QAbstractSocket::SocketError)));   // 显示错误信息
}

void chatWindow::GetFriendInfo(FriendInfo info)
{
    friendInfo = info;
    tcplink->disconnectfriendSocket();
    friendInfo.tcpSocket = info.tcpSocket;
    initSocket();
}

//打开文件
void chatWindow::on_openFileButton_clicked()
{
    SendFile.FileName = QFileDialog::getOpenFileName(this);
    if(!SendFile.FileName.isEmpty())
    {
        ui->sendFileButton->setEnabled(true);
        ui->clientStatusLabel->setText(tr("打开文件 %1 成功！").arg(SendFile.FileName));
    }
}

//连接并发送文件
void chatWindow::on_sendFileButton_clicked()
{
    ui->sendFileButton->setEnabled(false);
    SendFile.FinishedBytes = 0;
    ui->clientStatusLabel->setText(tr("连接中…"));
    tcpClient->connectToHost(friendInfo.node.hostAddr, /*getPortNumber(friendInfo.account)*/16666);
}

void chatWindow::StartTransmit()
{
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
    ui->clientStatusLabel->setText(tr("已连接"));
    SendFile.Buffer.resize(0);
}

void chatWindow::UpdateProgressBar(qint64 temp)
{
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
        ui->clientStatusLabel->setText(tr("传送文件 %1 成功").arg(SendFile.FileName));
        SendFile.File->close();
    }
}

void chatWindow::displayError(QAbstractSocket::SocketError)
{
    qDebug() << tcpClient->errorString();
    tcpClient->close();
    ui->clientProgressBar->reset();
    ui->clientStatusLabel->setText(tr("客户端就绪"));
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
    friendInfo.tcpSocket->write(block); // 发送数据
}
// 更新显示窗口
void chatWindow::appendShowLine()
{
    // 将接收到的信息显示在输出框
    QString datetime = getCurrentDateTime();
    QString temp = QString("<font size=\"4\" color=blue>%1    %2: </font>%3").arg(friendInfo.account).arg(datetime).arg(recieveString);
    ui->Show_message->append(temp); // 显示在输出框
}

// ，准备接收，接收消息
void chatWindow::readMessage()
{
    QDataStream in(friendInfo.tcpSocket);
    in.setVersion(QDataStream::Qt_5_3);
    if(blockSize == 0)
    {
        if(friendInfo.tcpSocket->bytesAvailable() < (int)sizeof(qint16))
        {
            return ;
        }
        in >> blockSize;
    }
    if(friendInfo.tcpSocket->bytesAvailable() < blockSize)
        return ;
    in >> recieveString;
    if (recieveString == "")
        return ;
    qDebug() << recieveString;
    blockSize = 0;  // 重新归零
    // 将接收到的消息显示在输出框
    chatWindow::appendShowLine();
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
    QTime time = QTime::currentTime();
    QDate date = QDate::currentDate();
    return QString("%1  %2").arg(date.toString(Qt::ISODate)).arg(time.toString(Qt::ISODate));
}

// 发送消息按下
void chatWindow::on_sendMsgButton_clicked()
{
    // 如果输入框为空，则忽略此消息，不予发送
    if(ui->Edit_massage->toPlainText().isEmpty())
        return ;
    // 获取输入框消息，并更新输出框
    QString tmpString = ui->Edit_massage->toHtml();    // 以Html格式发送
    ui->Edit_massage->clear();  // 输入框清空
    QString datetime = getCurrentDateTime();
    sendString = QString("<font size=\"4\" color=green>%1    %2: </font>%3").arg(tcplink->loginInfo.account).arg(datetime).arg(tmpString);   // 转为 HTML 账号-时间-消息
    ui->Show_message->append(sendString);   // 显示在输入窗口
    // 发送消息
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    out << (qint16)0;   // 预存数据大小
    out << tmpString;        // 发送消息
    out.device()->seek(0);  // 回到首部
    out << (qint16)(block.size() - sizeof(qint16));
    friendInfo.tcpSocket->write(block); // 发送数据
    qDebug() << tmpString;

}
