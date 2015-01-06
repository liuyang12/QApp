#include "video.h"
#include "ui_video.h"

Video::Video(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Video)
{
    ui->setupUi(this);

    //接收Socket初始化
    Receiver = new QUdpSocket(this);
    Receiver->bind(QHostAddress::Any,8888);
    connect(Receiver,SIGNAL(readyRead()),this,SLOT(ReadPeerFrame()));

    //发送Socket初始化
    //SetHostAddr(QString("183.173.168.53"));
    Sender = new QUdpSocket(this);
    Camera = NULL;
    FPStime = new QTimer(this);
    connect(FPStime,SIGNAL(timeout()),this,SLOT(ReadOwnFrame()));//定时读取帧

    OpenCamera();
}

Video::~Video()
{
    delete ui;
}

//打开摄像头
void Video::OpenCamera()
{
    Camera = cvCreateCameraCapture(0);  //打开摄像头
    FPStime->start(100);                //计时器，时间到则读取当前帧
}

//读取并显示自己的视频
void Video::ReadOwnFrame()
{
    ImageFrame = cvQueryFrame(Camera);
    //帧格式转换为QImage
    QImage img((const uchar*)ImageFrame->imageData,ImageFrame->width,ImageFrame->height,QImage::Format_RGB888);
    //OwnVideoLabel作为显示
    ui->OwnVideoLabel->setPixmap(QPixmap::fromImage(img));

    //图片以UDP形式发送
    QImage image = img.scaled(128,96,Qt::KeepAspectRatio);
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);
    image.save(&buffer,"PNG");
    qint64 res;
    if((res = Sender->writeDatagram(ba,ReceiverAddress,8888)) != ba.length())
    {
        return;
    }

}

//显示对方的视频
void Video::ReadPeerFrame()
{
    while(Receiver->hasPendingDatagrams())
    {
        QByteArray DataGram;
        DataGram.resize(Receiver->pendingDatagramSize());
        Receiver->readDatagram(DataGram.data(),DataGram.size());
        QImage image;
        image.loadFromData(DataGram);
        ui->PeerVideoLabel->setPixmap(QPixmap::fromImage(image));
    }
}

//拍照并指定图片存储位置
void Video::PhotoGraph()
{
    QImage image((const uchar*)ImageFrame->imageData,ImageFrame->width,ImageFrame->height,QImage::Format_RGB888);
    QString FileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                       ".",tr("Images (*.png)"));
    image.save(FileName,"PNG");
}

//关闭摄像头
void Video::CloseCamera()
{
    FPStime->stop();            //定时器停止
    cvReleaseCapture(&Camera);  //释放摄像头
}

//设置接收方地址
void Video::SetHostAddr(QString address)
{
    ReceiverAddress = QHostAddress(address);
}

//拍照功能
void Video::on_TakeCameraButton_clicked()
{
    PhotoGraph();
}

//关闭
void Video::on_CloseCameraButton_clicked()
{
    CloseCamera();
    emit closeMedia(2);
}


void Video::closeEvent(QCloseEvent *)
{
    CloseCamera();
    emit closeMedia(2);
}
