#include "chatwindow.h"
#include "ui_chatwindow.h"

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

}

chatWindow::~chatWindow()
{
    tcpClient->close();
    delete ui;
}

void chatWindow::GetFriendInfo(FriendInfo info)
{
    friendInfo = info;
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
    tcpClient->connectToHost(friendInfo.node.hostAddr,16666);
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
