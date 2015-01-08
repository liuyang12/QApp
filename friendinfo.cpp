#include "friendinfo.h"
#include "ui_friendinfo.h"
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>
#include "tcplink.h"

extern TCPLink * tcplink;   // TCPLink 全局变量外部申明

extern QSqlDatabase db;

friendinfo::friendinfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::friendinfo)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);     // 设置窗口无边框
    initStatus();
    connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(getCell(int,int)));

    ui->newgroup->setMouseTracking(true);
    ui->newgroup->setStyleSheet("QPushButton{color: rgb(3, 133, 255);}"
                                "QPushButton:hover{color: rgb(119, 178, 255);}"
                                );
    ui->confirm_button->setMouseTracking(true);
    ui->confirm_button->setStyleSheet("QPushButton{border-image: url(:/friendinfo/deepbule.jpg);color: rgb(255, 255, 255);}"
                                      "QPushButton:hover{border-image: url(:/friendinfo/bule.jpg);color: rgb(255, 255, 255);}"
                                      );
    ui->choose_tx->setMouseTracking(true);
    ui->choose_tx->setStyleSheet("QPushButton{border-image: url(:/friendinfo/bule.jpg);color: rgb(255, 255, 255);}"
                                 "QPushButton:hover{border-image: url(:/friendinfo/lessbule.jpg);color: rgb(255, 255, 255);}"
                                 );
    ui->gn_button->setMouseTracking(true);
    ui->gn_button->setStyleSheet("QPushButton{border-image: url(:/friendinfo/bule.jpg);color: rgb(255, 255, 255);}"
                                 "QPushButton:hover{border-image: url(:/friendinfo/lessbule.jpg);color: rgb(255, 255, 255);}"
                                 );
}

friendinfo::~friendinfo()
{
    delete ui;
}

//拖动窗口
void  friendinfo::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}
void  friendinfo::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() && Qt::LeftButton)
    {
        move(event->globalPos() - dragPosition);
        event->accept();
    }
}

//界面初始化
void friendinfo::initStatus()
{
    ui->nicheng->setText(tcplink->friendInfo.account); //默认昵称是学号
    ui->tx->setStyleSheet("border-image: url(:/friendinfo/tx.jpg);"); //初始头像

    //初始化QCombobox
    QSqlQuery p;
    p.exec("select * from groups");      // 指定查找数据库中的groups表
    while(p.next())
    {
        QString groupname = p.value(1).toString(); //读组名
        ui->comboBox->addItem(groupname);
    }
    //初始化tablewidget
    ui->tableWidget->setRowCount(3);     //设置行数为3
    ui->tableWidget->setColumnCount(4);   //设置列数为4

    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);//只能单选
    ui->tableWidget->verticalHeader()->setVisible(false);   //隐藏列表头
    ui->tableWidget->horizontalHeader()->setVisible(false); //隐藏行表头
    QLabel* lable[3][4];
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            lable[i][j] = new QLabel();
            if(i==0&&j==0) lable[i][j]->setStyleSheet("border-image: url(:/friendinfo/tx.jpg);");
            else lable[i][j]->setStyleSheet("border-image: url(:/friendinfo/tx"+QString::number(4*i+j-1)+".jpg);");
            ui->tableWidget->setCellWidget(i,j,lable[i][j]);
        }
    }
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    flag = false;
    ui->label_4->setVisible(flag);
    ui->groupname->setVisible(flag);
    ui->gn_button->setVisible(flag);
    flag_tx = false;
    ui->tableWidget->setVisible(flag_tx);
}


//新建分组
void friendinfo::on_newgroup_clicked()
{
    flag = !flag;
    ui->label_4->setVisible(flag);
    ui->groupname->setVisible(flag);
    ui->gn_button->setVisible(flag);
}

void friendinfo::on_gn_button_clicked()
{
    if(!ui->groupname->text().isEmpty())
    {
        ui->comboBox->addItem(ui->groupname->text());
        ui->comboBox->setCurrentIndex((ui->comboBox->count())-1);
    }
    flag = !flag;
    ui->label_4->setVisible(flag);
    ui->groupname->setVisible(flag);
    ui->gn_button->setVisible(flag);
}

void friendinfo::on_confirm_button_clicked()
{
    //点击确定后，更新groups表、friends表、返回qappwindow并刷新treewidget
    //插入语句：insert into 表名(字段列表) values(值列表)。如： insert into person(name, age) values(‘传智’,3)

    //注意要修改学号！注意不是直接添加一个分组
    QString currGroup;
    QSqlQuery qr;
    bool groupexist;
    groupexist = false; // 是否已存在该分组
    currGroup = ui->comboBox->currentText();
    qr.exec("select * from groups");    // 查找所有的组
    while(qr.next())
    {
        if(qr.value(1).toString() == currGroup)
        {
            QSqlQuery q;
            QString oldmembers;
            oldmembers = qr.value(2).toString();
            QStringList tempList = oldmembers.split(' ');
            for(int i = 0; i < tempList.size(); i++)
            {
                if(tempList[i] == tcplink->friendInfo.account)
                    continue;
            }
//            q.prepare("update friends set IP = :IP where account = :account");
            q.prepare("update groups set members = :members where name = :name");
            q.bindValue(":name", currGroup);
            q.bindValue(":members", oldmembers + " " + tcplink->friendInfo.account);    // 在最后增加一个成员，以空格间隔
            q.exec();
            qDebug() << oldmembers + " " + tcplink->friendInfo.account;
            groupexist =true;
        }
    }
    if(!groupexist)
    {
        // 在分组 groups 中增加该好友
        QString s_group = "insert into groups(name, members) values('"+ui->comboBox->currentText()+"','"+tcplink->friendInfo.account+"')";

        qDebug() << s_group;
        //QString s_friends = "insert into friends(account,nickname,avatar,_avatar,IP,status) values("+s1+","+s2+","+s3+","+s4+","+s5+","+s6+")";
        qr.exec(s_group);
    }
    tcplink->friendInfo.name = ui->nicheng->text(); // 昵称
    if(-1 == tcplink->findAccount(tcplink->friendInfo.account))
    {
        // 好友列表中不存在该好友
        tcplink->friendVect.push_back(tcplink->friendInfo); // 更新tcplink->friendVect
        tcplink->databaseInsert(tcplink->friendInfo);   // 更新数据库，插入到 friends 表中
        qDebug() << "更新好友信息成功:" << tcplink->friendInfo;
    }

    emit addfriendinfoSignal();     // 发出添加好友信号
    //p.exec(s_friends);

    this->close();
}

void friendinfo::on_choose_tx_clicked()
{
    //显示tablewidget
    flag_tx = !flag_tx;
    ui->tableWidget->setVisible(flag_tx);
}
// 设置好友头像
void friendinfo::getCell(int row,int col)
{
    //qDebug("%d,%d",row,col);
    QString s;
    if(row==0&col==0)
    {
        s = ":/friendinfo/tx.jpg";
    }
    else
        s = ":/friendinfo/tx"+QString::number(4*row+col-1)+".jpg";
    flag_tx = !flag_tx;
    ui->tableWidget->setVisible(flag_tx );
    ui->tx->setStyleSheet(QString("border-image: url(%1);").arg(s));
    tcplink->friendInfo.avatar = s; // 设置好友头像
    qDebug() << s;

}

