#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include "classConstant.h"

namespace Ui {
class login;
}

class login : public QDialog
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = 0);
    ~login();
    bool isConnected;           // status whether client is connected with server
    ServerNode serverNode;      // 服务器节点
    LoginInfo loginInfo;        // 登录信息
    QPoint dragPosition;

private slots:
    void newReply(qint32 replyKind);    // 收到新的回复信息
    void initStatus(void);              // 初始化状态
    void serverDisconnected(void);      // 服务器离线

    void reLogin(void);                 // 用户重新登录

    void on_EditNumber_textChanged(const QString &arg1);

    void on_EditPassword_textChanged(const QString &arg1);

    void on_buttonConfirm_clicked();

    void on_Button_ConfigServer_clicked();

//拖动窗口

    void mousePressEvent(QMouseEvent *event);

    void  mouseMoveEvent(QMouseEvent *event);

    void on_close_button_clicked();

private:
    Ui::login *ui;
signals:
    void loggedinSignal(void);      // 用户已经登录信号
    void disconnectedSignal(void);  // 服务器未连接
};

#endif // LOGIN_H
