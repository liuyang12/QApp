#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QDialog>
#include "classConstant.h"

namespace Ui {
class chatWindow;
}

class chatWindow : public QDialog
{
    Q_OBJECT

public:
    explicit chatWindow(QWidget *parent = 0);
    ~chatWindow();
    void initSocket(void );

private:
    Ui::chatWindow *ui;

    QTcpSocket *tcpClient;
    bool FileConnect;
    TSFile SendFile;

    FriendInfo friendInfo;  //好友信息
    QString sendString;     // 发送消息字符串
    QString recieveString;  // 接收消息字符串
    qint16 blockSize;       // 块大小

    QString getCurrentDateTime(void);   // 获取当前日期时间
protected:
    bool eventFilter(QObject *, QEvent *);  // 重写虚函数事件过滤

public slots:
    void GetFriendInfo(FriendInfo); //获取好友信息

private slots:
    void StartTransmit();
    void UpdateProgressBar(qint64);
    void displayError(QAbstractSocket::SocketError);
    void sendMessage(void);     // 发送好友消息
    void readMessage(void);     // 读取好友消息
    void displaySocketError(QAbstractSocket::SocketError);  // 显示 Socket 错误信息
    void appendShowLine(void/*Message &message*/);  // 更新显示窗口
    void on_openFileButton_clicked();
    void on_sendFileButton_clicked();
    void on_sendMsgButton_clicked();
//    void on_comboBox_activated(int index);

signals:
    void connectionFailedSignal();
};

#endif // CHATWINDOW_H
