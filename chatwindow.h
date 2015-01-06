#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QDialog>
#include <QDateTime>
#include "classConstant.h"

#include "video.h"

namespace Ui {
class chatWindow;
}

class chatWindow : public QDialog
{
    Q_OBJECT

public:
    explicit chatWindow(QVector<int> frNo, bool beStarter = false, QWidget *parent = 0);
    ~chatWindow();
    QPoint dragPosition;
    QString HeadString;     // 窗口标题
    bool beStarter;         // 群聊的发起者
    QDateTime lastSpeakTime; // 上一个聊天时刻
    QString lastSpeaker;    // 上一个聊天者
    QVector<int> friendNo;  // 聊天窗口中的所有好友的编号，通过这个与 friendVect[] 获取好友的TCPSocket 进而进行聊天，默认其中包含自己，但是不显式表示出来 friendNo[0] 是第一个好友的编号，而不是自己
    void initWindowHead(void );     // 设置窗口标题和头像
    void initSocket(void );     // 初始化 TCPSocket 通信
    bool operator ==(const chatWindow &chat);   // 重载等号运算符，如果两个窗口 friendNo 一致（不考虑序号）则两个窗口是同一个窗口

private:
    Ui::chatWindow *ui;

    QTcpSocket *tcpClient;  //传送文件Socket
    bool FileConnect;       //传送文件链接确认
    TSFile SendFile;        //文件信息

    Speech TranSpeech;      //语音传输

    FriendInfo friendInfo;  //好友信息
    QString sendString;     // 发送消息字符串
    QString recieveString;  // 接收消息字符串
    qint16 blockSize;       // 块大小

    QString getCurrentDateTime(void);   // 获取当前日期时间

    Video *videoTrans;
protected:
    bool eventFilter(QObject *, QEvent *);  // 重写虚函数事件过滤

public slots:
    void GetFriendInfo(FriendInfo); //获取好友信息
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void StartTransmit();
    void UpdateProgressBar(qint64);
    void displayError(QAbstractSocket::SocketError);
    void sendMessage(void);     // 发送好友消息
    void readMessage(void);     // 读取好友消息
    void displaySocketError(QAbstractSocket::SocketError);  // 显示 Socket 错误信息
    void appendShowLine(QString &account);  // 更新显示窗口
    void on_openFileButton_clicked();
    void on_sendMsgButton_clicked();
    void on_closebutton_clicked();
    void on_minButton_clicked();
//    void on_comboBox_activated(int index);

    void on_SpeechButton_clicked();

    void SpeechConnection();                //语音连接
    void SpeechTransfer();                  //语音传输
    void readSpeech();                      //读取语音
    void SpeechServerClose();               //关闭语音

    void MediaOpen(int choice);             //开启语音&视频

    void on_VideoButton_clicked();

signals:
    void connectionFailedSignal();
};
#ifndef FRIENDNOEQUAL
#define FRIENDNOEQUAL
inline bool friendNoEqual(QVector<int> friendNo_a, QVector<int> friendNo_b);
#endif

#endif // CHATWINDOW_H
