#include "messdialog.h"
#include "ui_messdialog.h"

MessDialog::MessDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

    //定时器
    timeShow = new QTimer(this);
    timeStay = new QTimer(this);
    timeClose = new QTimer(this);
    connect(timeShow,SIGNAL(timeout()),this,SLOT(MessDialogShow()));
    connect(timeStay,SIGNAL(timeout()),this,SLOT(MessDialogStay()));
    connect(timeClose,SIGNAL(timeout()),this,SLOT(MessDialogClose()));
    DynamicShow();
}

MessDialog::~MessDialog()
{
    delete ui;
}

void MessDialog::DynamicShow()
{
    QDesktopWidget *deskTop = QApplication::desktop();
    deskRect = deskTop->availableGeometry();
    Location.setX(deskRect.width() - rect().width() - 1);
    Location.setY(deskRect.height() - rect().height());
    move(Location.x(),deskRect.height() - 1);
    show();
    timeShow->start(5);
}

//淡入效果
void MessDialog::MessDialogShow()
{
    static int beginY = QApplication::desktop()->height();
    beginY--;
    move(Location.x(),beginY);
    if(beginY <= Location.y())
    {
        timeShow->stop();
        timeStay->start(1000);
    }
}

//停留效果
void MessDialog::MessDialogStay()
{
    static int timeCount = 0;
    timeCount++;
    if(timeCount >= 9)
    {
        timeStay->stop();
        timeClose->start(200);
    }
}

//淡出效果
void MessDialog::MessDialogClose()
{
    static double trTimes = 1.0;
    trTimes -= 0.1;
    if(trTimes <= 0.0)
    {
        timeClose->stop();
        emit close();
    }
    else
        setWindowOpacity(trTimes);
}

void MessDialog::on_CloseButton_clicked()
{
    close();
}

void MessDialog::on_WorkButton_clicked()
{
    //查看
}

//设置显示内容
void MessDialog::SetMessage(QString Title,QString MainDoc,QString YesButton,QString NoButton)
{
    ui->Title_label->setText(Title);
    ui->DocBrowser->setText(MainDoc);
    ui->WorkButton->setText(YesButton);
    ui->CloseButton->setText(NoButton);
}
