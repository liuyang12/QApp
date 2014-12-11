#ifndef CLASSCONSTANT_H
#define CLASSCONSTANT_H
// 宏定义以及类与结构体的定义
#include <QtCore>
#include <QtGui>
#include <QtWidgets>
//#include <QHostAddress>
// 服务器端 ServerNode 节点类
enum STATUS{    // 状态
    OFFLINE,    // 离线 = 0
    ONLINE,     // 在线 = 1
    TIMEOUT     // 连接超时 = 2
};

struct ServerNode{
    QString hostAddr;       // 服务器地址
    quint16 hostPort;       // 服务器端口
    ServerNode()
    {
        hostPort = 0; /*  */
    }
};
// 用户信息类
struct UserInfo{
    QString account;        // 账号
    QString password;       // 密码
    QString nickname;       // 昵称
    int avatarNumber;       // 头像编号
    int status;              // 登录状态
    QString mobileNumber;   // 手机号码
    QString phoneNumber;    // 电话号码
    QString birthday;       // 生日
    QString city;           // 城市
    QString aboutMyself;    // 个人描述
    UserInfo()
    {
        avatarNumber = 0;
        status =  0;
    }
    //重载输入输出操作符
    friend QDataStream & operator<< (QDataStream &qos, const UserInfo &pi)
    {
        qos << pi.account << pi.password << pi.nickname << pi.avatarNumber
            << pi.status << pi.mobileNumber
            << pi.phoneNumber << pi.birthday<< pi.city << pi.aboutMyself;
        return qos;
    }
    friend QDataStream & operator>> (QDataStream &qis, UserInfo &pi)
    {
        qis >> pi.account >> pi.password >> pi.nickname >> pi.avatarNumber
            >> pi.status >> pi.mobileNumber
            >> pi.phoneNumber >> pi.birthday >> pi.city >> pi.aboutMyself;
        return qis;
    }

};
// 用户登录信息类
struct LoginInfo{
    QString account;
    QString password;
    int status;
    LoginInfo()
    {
        status = 0;
    }
    friend QDataStream & operator<< (QDataStream &qos, LoginInfo &li)
    {
        qos << li.account << li.password << li.status;
        return qos;
    }
    friend QDataStream & operator>> (QDataStream &qis, LoginInfo &li)
    {
        qis >> li.account >> li.password >> li.status;
        return qis;
    }
};
// 好友信息类
struct FriendInfo
{
    QString account;        // 好友账号
    QString name;           // 好友名字
    int avatarNumber;       // 好友头像编号
    int status;             // 好友在线状态
    QString about;          // 好友信息
    int friendKind;        //好友类型
    QString remark;			//好友备注

    FriendInfo()
    {	avatarNumber = status = friendKind = 0;	}

    friend QDataStream & operator<< (QDataStream &qos, const FriendInfo &fi)
    {
        qos << fi.account << fi.name << fi.avatarNumber << fi.status
            << fi.about << fi.friendKind << fi.remark;
        return qos;
    }
    friend QDataStream & operator>> (QDataStream &qis, FriendInfo &fi)
    {
        qis >> fi.account >> fi.name >> fi.avatarNumber >> fi.status
            >> fi.about >> fi.friendKind >> fi.remark;
        return qis;
    }
};
// 会话消息类
struct Message
{
    int kind;				//消息类型
    QString sender;			//发送者
    QString receiver;		//接收者
    QString text;			//消息内容

    Message()
    {	kind = 0;	}
    friend QDataStream & operator<< (QDataStream &qos, const Message &mes)
    {
        qos << mes.kind << mes.sender << mes.receiver << mes.text;
        return qos;
    }
    friend QDataStream & operator>> (QDataStream &qis, Message &mes)
    {
        qis >> mes.kind >> mes.sender >> mes.receiver >> mes.text;
        return qis;
    }
};

#endif // CLASSCONSTANT_H
