#include "login.h"
#include "ui_login.h"
#include <QTextCodec>
//#include "qtextcodec.h"
#include <QMessageBox>
#include <QDebug>

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    // 设置语言编码格式
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("system"));
    // 服务器IP地址、端口初始化
    isConnected = false;
    login::hostAddr = QHostAddress("166.111.180.60");
    login::hostPort = 8000;
    login::tcpSocket = new QTcpSocket(this);

    connect(tcpSocket, SIGNAL(connected()), this, SLOT(sendRequest()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readResult()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(ServerDisconnected()));
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

    // 登录验证初始化
//    login::LogNumber = tr("");
//    login::LogPassword = tr("");
    this->ui->EditNumber->setText(tr("2012011"));
    this->ui->EditPassword->setText(tr(""));
    // 输入框响应回车
    connect(ui->EditNumber, SIGNAL(returnPressed()), this, SLOT(on_buttonConfirm_clicked()));
    connect(ui->EditPassword, SIGNAL(returnPressed()), this, SLOT(on_buttonConfirm_clicked()));

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
    isConnected = true;
    qDebug() << LogInfo;
    tcpSocket->write(LogInfo.toStdString().c_str());
//    QByteArray block;
//    QDataStream out(&block, QIODevice::WriteOnly);
//    out.setVersion(QDataStream::Qt_5_2);

//    out << (quint16)0;
////    out << (qint16)0;
//    // 向服务器发送字符串 LogNumber_LogPassword
//    out << LogNumber << "_" << LogPassword;

//    out.device()->seek(0);
//    out << (quint16)(block.size() - sizeof(quint16));
//    tcpSocket->write(block);
////    QMessageBox::information(this, tr("info"), block.data(), QMessageBox::Ok);
//    if(!isConnected)
//        isConnected = true;
////    QMessageBox::information(this, tr("info"), tr("请求发送成功"), QMessageBox::Ok);

}
// 读取服务器端的结果
void login::readResult()
{
    QByteArray qba;
    qba = tcpSocket->readAll();
    Reply =  QVariant(qba).toString();
//    QMessageBox::information(this, tr("info"), tmp, QMessageBox::Ok);
//    QDataStream in(tcpSocket);
//    in.setVersion(QDataStream::Qt_5_2);

//    if(0 == blockSize)
//    {
//        if(tcpSocket->bytesAvailable() < (int)sizeof(quint16))
//        {
//            return ;
//        }
//        in >> blockSize;
//    }
//    if(tcpSocket->bytesAvailable() < blockSize)
//    {
//        return ;
//    }
//    in >> Reply;
    qDebug() << Reply;
    // 用户登录成功，服务器返回 "lol"
    // 用户账号格式错误，服务器返回 "Incorrect No."
    // 密码输入或密码错误，服务器返回 "Please send the correct message."
    // QMessageBox 点 Ok 之后程序异常退出
//    QMessageBox::information(NULL, tr("info"), Reply);
    if(Reply == "lol")
    {
        qDebug() << "用户" << LogNumber << "登录成功";
//        QTextCodec::setCodecForTr( QTextCodec::codecForName("GBK") );
//        QMessageBox::information(this, tr("info"), tr("用户登录成功"), QMessageBox::Yes);
        accept();
        this->close();
    }
    else if(Reply == "Incorrect No.")
    {
        qDebug() << "用户" << LogNumber << "账号格式错误\n请重新输入。。。";
//        QMessageBox::information(this, tr("info"), tr("账户格式错误\n请重新输入。。。"));
    }
    else if(Reply == "Please send the correct message.")
    {
        qDebug() << "用户" << LogNumber << "账号或密码错误\n请重新输入。。。";
//        QMessageBox::information(this, tr("info"), tr("账号或密码错误\n请重新输入。。。"));
    }
    else if(Reply == "")
    {
        qDebug() << "用户" << LogNumber << "未收到用户请求结果，请检查：\n1. 用户网络状态\n2. 服务器地址和端口号（默认166.111.180.60:8000）";
//        QMessageBox::information(this, tr("info"), tr("未收到用户请求结果，请检查：\n1. 用户网络状态\n2. 服务器地址和端口号（默认166.111.180.60:8000）"));
    }
    else
    {
        qDebug() << "用户" << LogNumber << "404 not found!\ndefault message";
//        QMessageBox::information(this, tr("info"), tr("404 not found!\ndefault message"));
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

//void login::on_buttonConfirm_accepted()
//{

//}

void login::on_EditNumber_textChanged(const QString &arg1)
{
    LogNumber = arg1;       // 更新输入框用户名
    // 响应回车
//    connect(ui->EditNumber, SIGNAL(QLineEdit::returnPressed()), this, SLOT(on_buttonConfirm_accepted()));
}

void login::on_EditPassword_textChanged(const QString &arg1)
{
    LogPassword = arg1;     // 更新输入框密码
//    ui->EditPassword->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    // 响应回车
//    connect(ui->EditPassword, SIGNAL(QLineEdit::returnPressed()), this, SLOT(on_buttonConfirm_accepted()));
}

void login::on_buttonConfirm_clicked()
{
    /// 登录验证
    if(ui->EditNumber->text()/*.trimmed()*/ == tr("") || ui->EditPassword->text()/*.trimmed()*/ == tr(""))  // 判断为空或没有输入，应该不予登录
    {
        qDebug() << "提示：用户名/密码不能为空！\n请重新输入。。。";
//        QMessageBox::warning(NULL, tr("Warning"), tr("提示：用户名/密码不能为空！\n请重新输入。。。")/*, QMessageBox::Ok*/);
        this->ui->EditNumber->setText(tr("2012011"));
        this->ui->EditPassword->setText(tr(""));
        return ;
    }
    else
    {
        LogNumber = ui->EditNumber->text()/*.trimmed()*/;
        LogPassword = ui->EditPassword->text()/*.trimmed()*/;
        LogInfo = LogNumber + tr("_") + LogPassword;
        login::newConnect();

    }
}

void login::on_pushButton_clicked()
{
    this->reject();
    this->close();
}
