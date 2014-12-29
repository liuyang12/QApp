#include "login.h"
#include "ui_login.h"
#include <QTextCodec>
//#include "qtextcodec.h"
#include <QMessageBox>
#include <QDebug>
#include "qappwindow.h"    // 主界面窗口，登录成功后弹出
#include <QMovie>
#include <QtGui>
#include "tcplink.h"

extern TCPLink *tcplink;           // tcplink 全局变量

login::login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    // 设置语言编码格式
//    QTextCodec::setCodecForLocale(QTextCodec::codecForName("system"));
    // 服务器IP地址、端口初始化
    serverNode.hostAddr = "166.111.180.60";
    serverNode.hostPort = 8000;

    //QMovie*movie=new QMovie(":picture/2.gif");
    tcplink = new TCPLink(serverNode, this);
    ui->Edit_ServerAddr->setText(serverNode.hostAddr);
    ui->Edit_ServerPort->setText("8000");
    ui->Edit_ServerAddr->setDisabled(true);
    ui->Edit_ServerPort->setDisabled(true);

    //鼠标移动到按钮上按钮图片变化
    ui->Button_ConfigServer->setMouseTracking(true);
    ui->Button_ConfigServer->setStyleSheet("QPushButton{border-image: url(:/picture/kb.png);background-image: url(:/picture/kb.png);}"
                                           "QPushButton:hover{border-image: url(:/picture/kb.png);background-color: qlineargradient(spread:pad, x1:0.555682, y1:0.222, x2:0.55, y2:1, stop:0.482955 rgba(213, 0, 10, 232), stop:1 rgba(213, 0, 10, 82));}"
                                           );
    ui->close_button->setMouseTracking(true);
    ui->close_button->setStyleSheet("QPushButton{border-image: url(:/picture/kb.png);background-image: url(:/picture/kb.png);}"
                                    "QPushButton:hover{border-image: url(:/picture/kb.png);background-color: qlineargradient(spread:pad, x1:0.555682, y1:0.222, x2:0.55, y2:1, stop:0.482955 rgba(213, 0, 10, 232), stop:1 rgba(213, 0, 10, 82));}"
                                    );
    ui->label->setVisible(false);
    ui->label_2->setVisible(false);
    ui->Edit_ServerAddr->setVisible(false);
    ui->Edit_ServerPort->setVisible(false);


    //ui->giflable->setMovie(movie);//gif显示
    //movie->start();
    this->setWindowFlags(Qt::FramelessWindowHint);     // 设置窗口无边框
    this->ui->EditNumber->setText(tr("2012011"));
    this->ui->EditPassword->setText(tr(""));
    this->ui->EditPassword->setEchoMode(QLineEdit::Password);
    // 登录界面初始设置
    login::initStatus();
    // 输入框响应回车
    connect(ui->EditNumber, SIGNAL(returnPressed()), this, SLOT(on_buttonConfirm_clicked()));
    connect(ui->EditPassword, SIGNAL(returnPressed()), this, SLOT(on_buttonConfirm_clicked()));

    connect(tcplink, SIGNAL(newReplySignal(qint32)), this, SLOT(newReply(qint32)));
    connect(tcplink, SIGNAL(connectionFailedSignal()), this, SLOT(initStatus()));
    connect(tcplink, SIGNAL(disconnectedSignal()), this, SLOT(serverDisconnected()));

}

login::~login()
{
    delete ui;
}
// 初始化状态
void login::initStatus()
{
    // 相关按键的初始化状态 setEnable(true/false)
}
// 服务器端断开连接
void login::serverDisconnected()
{
    emit disconnectedSignal();
}

//拖动窗口
void  login::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
//    if (event->button() == Qt::RightButton)
//        close();//如果是右键，则关闭窗体
}
void  login::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() && Qt::LeftButton)
    {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}



void login::on_EditNumber_textChanged(const QString &arg1)
{
//    LogNumber = arg1;       // 更新输入框用户名
    // 响应回车
//    connect(ui->EditNumber, SIGNAL(QLineEdit::returnPressed()), this, SLOT(on_buttonConfirm_accepted()));
}

void login::on_EditPassword_textChanged(const QString &arg1)
{
    // TODO: 做密码的输入连续隐藏的效果
//    LogPassword = arg1;     // 更新输入框密码
//    ui->EditPassword->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    // 响应回车
//    connect(ui->EditPassword, SIGNAL(QLineEdit::returnPressed()), this, SLOT(on_buttonConfirm_accepted()));
}

void login::on_buttonConfirm_clicked()
{
    /// 首先验证服务器IP地址和端口号是否正确
    if(ui->Edit_ServerAddr->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Ip address's cannot be empty. Please try again"));
        return;
    }
    if(!ipRegExp.exactMatch(ui->Edit_ServerAddr->text()))
    {
        QMessageBox::critical(this, tr("Error"),
            tr("Ip address's Format is wrong. Please try again"));
        return;
    }
    if(ui->Edit_ServerPort->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"),
            tr("port number's cannot be empty. Please try again"));
        return;
    }
//    if(!portRegExp.exactMatch(ui->Edit_ServerAddr->text()))
//    {
//        QMessageBox::critical(this, tr("Error"),
//            tr("port number's Format is wrong. Please try again"));
//        return;
//    }
    /// 登录验证
    if(ui->EditNumber->text().isEmpty() || ui->EditNumber->text() == tr("2012011"))
    {
        qDebug() << "提示：用户名不能为空！\n请重新输入。。。";
        QMessageBox::warning(NULL, tr("Warning"), tr("提示：用户名不能为空！\n请重新输入。。。")/*, QMessageBox::Ok*/);
        this->ui->EditNumber->setText(tr("2012011"));
//        this->ui->EditPassword->setText(tr(""));
        return ;
    }
    else if(ui->EditPassword->text().isEmpty())  // 判断为空或没有输入，应该不予登录
    {
        qDebug() << "提示：密码不能为空！\n请重新输入。。。";
        QMessageBox::warning(NULL, tr("Warning"), tr("提示：密码不能为空！\n请重新输入。。。")/*, QMessageBox::Ok*/);
//        this->ui->EditNumber->setText(tr("2012011"));
//        this->ui->EditPassword->setText(tr(""));
        return ;
    }
    else
    {
        serverNode.hostAddr = ui->Edit_ServerAddr->text();
        serverNode.hostPort = ui->Edit_ServerPort->text().toInt();

        loginInfo.account = ui->EditNumber->text();
        loginInfo.password = ui->EditPassword->text();

        tcplink->serverNode = serverNode;
        tcplink->requestKind = LOGIN;
        tcplink->loginInfo = loginInfo;
        tcplink->loginRequest(tcplink->loginInfo);
        setWindowTitle("logging in...");
        ui->buttonConfirm->setEnabled(false);
    }
}
void login::newReply(qint32 replyKind)
{
    //TODO: check connection with server
    /*      */
    QAppWindow *qapp;
    switch (replyKind) {
    case LOGIN_SUCCESS:
        loginInfo.status = ONLINE;
        qDebug() << "用户" << loginInfo.account << "登录成功";
//        QTextCodec::setCodecForTr( QTextCodec::codecForName("GBK") );
        QMessageBox::information(this, tr("info"), tr("用户登录成功"), QMessageBox::Yes);
        emit loggedinSignal();
        // 调用主界面
        qapp = new QAppWindow(/*(tcplink, */NULL);
        // 连接重新登录信号
        connect(qapp, SIGNAL(reLoginSignal()), this, SLOT(reLogin()));
        ui->buttonConfirm->setEnabled(true);
        qapp->show();
        this->close();
        break;
    case LOGIN_NO_ACCOUNT:
        loginInfo.status = OFFLINE;
        qDebug() << "用户" << loginInfo.account << "账号格式错误\n请重新输入。。。";
        QMessageBox::information(this, tr("info"), tr("账户格式错误\n请重新输入。。。"));
        break;
    case LOGIN_WRONG_INPUT:
        loginInfo.status = OFFLINE;
        qDebug() << "用户" << loginInfo.account << "账号或密码错误\n请重新输入。。。";
        QMessageBox::information(this, tr("info"), tr("账号或密码错误\n请重新输入。。。"));
        break;
    case HAVE_LOGINED:
        // 监测用户是否已经登录
        break;
    default:
        break;
    }
}

void login::reLogin()
{
    tcplink = new TCPLink(serverNode);
    connect(tcplink, SIGNAL(newReplySignal(qint32)), this, SLOT(newReply(qint32)));
    connect(tcplink, SIGNAL(connectionFailedSignal()), this, SLOT(initStatus()));
    connect(tcplink, SIGNAL(disconnectedSignal()), this, SLOT(serverDisconnected()));
    this->show();
}

void login::on_close_button_clicked()
{
    ui->Button_ConfigServer->raise();
    this->reject();
    this->close();
}

void login::on_Button_ConfigServer_clicked()
{
    ui->label->setVisible(true);
    ui->label_2->setVisible(true);
    ui->Edit_ServerAddr->setVisible(true);
    ui->Edit_ServerPort->setVisible(true);
    // 允许设置服务器地址
    ui->Edit_ServerAddr->setDisabled(false);
    ui->Edit_ServerPort->setDisabled(false);
}
