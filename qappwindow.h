#ifndef QAPPWINDOW_H
#define QAPPWINDOW_H

#include <QMainWindow>
#include "tcplink.h"

namespace Ui {
class QAppWindow;
}

class QAppWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QAppWindow(TCPLink* tcplink, QWidget *parent = 0);
    ~QAppWindow();

private slots:
    void newReply(qint32 replyKind);    // 收到新的回复信息
    void initStatus(void);              // 初始化状态
//    void connectedStatus(void);         // 连接建立后状态
//    void connectionFailed(void);        // 与服务器连接失败
    void serverDisconnected(void);      // 服务器离线

    void on_Button_queryFriend_clicked();

    void on_action_Logout_triggered();

private:
    Ui::QAppWindow *ui;

    TCPLink *tcplink;
signals:
    void reLoginSignal(void);     // 重新登录请求
    void disconnectedSignal(void);
};

#endif // QAPPWINDOW_H
