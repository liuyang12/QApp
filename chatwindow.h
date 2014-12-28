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

private:
    Ui::chatWindow *ui;

    QTcpSocket *tcpClient;

    TSFile SendFile;
    //TSFile ReceiveFile;

    FriendInfo friendInfo;  //好友信息

public slots:
    void GetFriendInfo(FriendInfo); //获取好友信息

private slots:
    void StartTransmit();
    void UpdateProgressBar(qint64);
    void displayError(QAbstractSocket::SocketError);
    void on_openFileButton_clicked();
    void on_sendFileButton_clicked();
};

#endif // CHATWINDOW_H
