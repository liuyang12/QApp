#include "login.h"
#include "ui_login.h"
#include <QTextCodec>
#include <QMessageBox>

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    // 设置语言编码格式
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
    // 服务器IP地址、端口初始化
    isConnected = false;
    login::hostAddr = QHostAddress(tr("166.111.180.60"));
    login::hostPort = 8000;
    login::tcpSocket = new QTcpSocket(this);
//    tcpSocket->abort();
//    tcpSocket->connectToHost(hostAddr, hostPort, QTcpSocket::ReadWrite);
//    udpSocket->bind(hostPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
//    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    // 槽与信号相连
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(sendRequest()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readResult()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(ServerDisconnected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));


    // 登陆验证初始化
    login::LogNumber = tr("");
    login::LogPassword = tr("");
    this->ui->EditNumber->setText(tr("2012011"));
    this->ui->EditPassword->setText(tr(""));
    // 输入框响应回车
    connect(ui->EditNumber, SIGNAL(QLineEdit::returnPressed()), this, SLOT(on_buttonConfirm_accepted()));
    connect(ui->EditPassword, SIGNAL(QLineEdit::returnPressed()), this, SLOT(on_buttonConfirm_accepted()));

}

login::~login()
{
    delete ui;
}
// TCP初始化连接
void login::newConnect()
{
    login::blockSize = 0;
    if(!isConnected)
    {
        tcpSocket->abort();
        tcpSocket->connectToHost(hostAddr, hostPort);
    }
    else
    {
        sendRequest();
    }
}
// 发送请求，TCP初始化
void login::sendRequest()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_8);

    out << (qint16)0;
    // 向服务器发送字符串 LogNumber_LogPassword
    out << LogNumber << "_" << LogPassword;

    out.device()->seek(0);
    out << (qint16)(block.size() - sizeof(qint16));
    tcpSocket->write(block);
    if(!isConnected)
        isConnected = true;

}
// 读取服务器端的结果
void login::readResult()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_4_8);

    if(0 == blockSize)
    {
        if(tcpSocket->bytesAvailable() < (int)sizeof(qint16))
            return ;
        in >> blockSize;
    }
    if(tcpSocket->bytesAvailable() < blockSize)
        return ;
    in >> Reply;
    if("lol" == Reply)
    {
        QMessageBox::information(this, tr("info"), tr("用户登陆成功"), QMessageBox::Ok);
        this->accept();
    }
    else
    {
        QMessageBox::warning(this, tr("warning"), tr("用户登陆失败\n提示：用户名或密码错误\n请重新输入。。。"), QMessageBox::Ok);
        this->ui->EditNumber->setText(tr("2012011"));
        this->ui->EditPassword->setText(tr(""));
        this->reject();

    }
    blockSize = 0;

}
// 服务器端断开连接
void login::ServerDisconnected()
{
    emit disconnectedSignal();
}
// 连接错误
void login::displayError(QAbstractSocket::SocketError socketError)
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

//// 处理接收到的信息
//void login::processPendingDatagrams()
//{

//}

void login::on_buttonConfirm_accepted()
{
    /// 登陆验证
    if(this->ui->EditNumber->text().trimmed() == tr("") || this->ui->EditPassword->text().trimmed() == tr(""))  // 判断为空或没有输入，应该不予登陆
    {
        QMessageBox::warning(0, tr("Warning"), tr("提示：用户名/密码不能为空！\n请重新输入。。。"), QMessageBox::Ok);
        this->ui->EditNumber->setText(tr("2012011"));
        this->ui->EditPassword->setText(tr(""));
        return ;
    }
    else
    {
        LogNumber = this->ui->EditNumber->text().trimmed();
        LogPassword  =this->ui->EditPassword->text().trimmed();
        login::newConnect();

    }
}

void login::on_EditNumber_textChanged(const QString &arg1)
{
    LogNumber = arg1;       // 更新输入框用户名
    // 响应回车
//    connect(ui->EditNumber, SIGNAL(QLineEdit::returnPressed()), this, SLOT(on_buttonConfirm_accepted()));
}

void login::on_EditPassword_textChanged(const QString &arg1)
{
    LogPassword = arg1;     // 更新输入框密码
    ui->EditPassword->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    // 响应回车
//    connect(ui->EditPassword, SIGNAL(QLineEdit::returnPressed()), this, SLOT(on_buttonConfirm_accepted()));
}
