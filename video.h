#ifndef VIDEO_H
#define VIDEO_H

#include <QWidget>
#include <QImage>
#include <QTimer>
#include <QBuffer>
#include <QFileDialog>
#include <QFile>
#include <QtNetwork/QUdpSocket>

#include <opencv.hpp>

namespace Ui {
class Video;
}

class Video : public QWidget
{
    Q_OBJECT

public:
    explicit Video(QWidget *parent = 0);
    ~Video();

    void SetHostAddr(QString address);  //设置IP地址

private:
    Ui::Video *ui;

    QUdpSocket *Sender;                 //发送Socket
    QUdpSocket *Receiver;               //接收Socket

    QTimer *FPStime;                    //定时器
    CvCapture *Camera;                  //视频获取
    IplImage *ImageFrame;               //存放图像帧

    QHostAddress ReceiverAddress;

private slots:
    void OpenCamera();                  //开启摄像头
    void ReadOwnFrame();                //读取自己当前帧
    void ReadPeerFrame();               //读取对方当前帧
    void CloseCamera();                 //关闭摄像头
    void PhotoGraph();                  //拍照功能
    void on_TakeCameraButton_clicked(); //拍照
    void on_CloseCameraButton_clicked();//关闭按钮

    void closeEvent(QCloseEvent *);                  //重载关闭

signals:
    void closeMedia(int choice);                  //关闭聊天窗口中的TCP连接
};

#endif // VIDEO_H
