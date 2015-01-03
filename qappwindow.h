#ifndef QAPPWINDOW_H
#define QAPPWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
//#include <QTimer>
//#include "tcplink.h"
#include "classConstant.h"

#include "chatwindow.h" // 包含所有聊天窗口，并对所有聊天窗口进行管理

namespace Ui {
class QAppWindow;
}

class QAppWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QAppWindow(/*TCPLink* tcplink,*/ QWidget *parent = 0);
    ~QAppWindow();

    void newTCPConnect(void);           // 建立与服务器之间的连接
    QPoint dragPosition;//拖动窗口

    bool flag;
    void fetchDatabase(void);           // 从数据库拉取数据，更新 friendVect[]
    int findChatWindow(const QVector<int> friendNo);  // 查找所有的聊天窗口，如果找到相一致的聊天窗口返回相应的窗口序号，否则返回 -1 （注意：friendNo 不一定是好友编号的序号不一定一致）

private slots:
    void newReply(qint32 replyKind);    // 收到新的回复信息
    void travelsalReply(qint32 replyKind);  // 好友遍历回复
    void initStatus(void);              // 初始化状态
////    void connectedStatus(void);         // 连接建立后状态
////    void connectionFailed(void);        // 与服务器连接失败
    void serverDisconnected(void);      // 服务器离线

//    void newListen(void);               // 建立 TCP 监听事件
//    void acceptConnection(void);        // 接受客户端连接
//    void displayError(QAbstractSocket::SocketError);    // 显示错误信息
//    void sendRequest();                 // 向服务器发送请求
//    void recieveData(void);             // 接受来自服务器端的数据

    void on_Button_queryFriend_clicked();

    void on_action_Logout_triggered();

    void on_OkPushButton_clicked();

//文件传输
    void acceptConnection();                                    //建立连接
    void FileReceive();                                         //接收数据并更新进度条
    void displayError(QAbstractSocket::SocketError socketError);//显示错误

//    void on_pushButton_clicked();

    void on_closeButton_clicked();

    void on_qappminButton_clicked();

    //拖动窗口

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    //建QTreewidget
    void build_tree();

//    void on_label_linkActivated(const QString &link);

    void on_addfriend_clicked();

private:
    Ui::QAppWindow *ui;

    QVector<chatWindow *> chatVect;   // 包含所有的聊天窗口
//    void newchat();

    QTcpServer *tcpServer;      // TCP Server
    QTcpSocket *tcpClient;      // TCP Socket
//    QTimer *timer;              // 计时器
////    QTcpSocket *tcpSender;      // TCP Sender

//    TCPLink *tcplink;
    TSFile ReceiveFile;
    bool FileConnect;
    QDialog                 *ReceiveDialog;
    QProgressBar            *ServerProgressBar;
    QPushButton             *OkPushButton;
    QVBoxLayout             *pLayout;

signals:
    void reLoginSignal(void);     // 重新登录请求
    void disconnectedSignal(void);

    void FriendInfoSignal(FriendInfo);//好友信息
};

#endif // QAPPWINDOW_H
