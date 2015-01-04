#ifndef CLASSCONSTANT_H
#define CLASSCONSTANT_H
// 宏定义以及类与结构体的定义
//#include <QtCore>
//#include <QtGui>
#include <QtWidgets>
#include <QMessageBox>
#include <QTcpSocket>
#include <QDebug>   // Qt Debug
#include <QtNetwork>
#include <QFile>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioOutput>
//#include <QHostAddress>
/// 服务器端 ServerNode 节点类
enum STATUS{    // 状态
    OFFLINE = 0,    // 离线 = 0
    ONLINE = 1,     // 在线 = 1
    TIMEOUT = 2,     // 连接超时 = 2
    IPUPDATED = 3   // IP 地址变化
};
enum REQUEST{   // 请求
    LOGIN = 10,     // 登录 = 10
    QUERY = 11,     // 查找 = 11
    LOGOUT = 12,    // 登出 = 12
    MESSAGE = 13,   // 发送信息 = 13
    ADD_FRIEND = 14,  // 发送好友请求 = 14
    GET_FRIEND = 15,     // 好友请求
    TRAVELSAL = 16,     // 遍历查询所有好友状态
    CONNECT = 17,       // 发起连接
    START_CHAT = 18,     // 开始聊天请求
    GROUP_CHAT = 19     // 群聊
};
enum REPLY{     // 回复
    NO_REPLY = 99,          // 无回复
    // 登录回复
    LOGIN_SUCCESS = 20,     // 登录成功 = 20
    LOGIN_NO_ACCOUNT = 21,  // 用户名格式错误
    LOGIN_WRONG_INPUT = 22, // 用户名或密码错误
    HAVE_LOGINED = 23,        // 已经登录
    // 查找好友回复
    FRIEND_OFFLINE = 25,    // 好友未在线
    FRIEND_NO_ACCOUNT = 27, // 无此账号
    FRIEND_ONLINE = 26,     // 好友在线
    // 添加好友回复
    ADDFRIEND_SUCCESS = 30, // 成功添加好友
    ADDFRIEND_DENY = 31,    // 好友拒绝
    HAVE_ADDED = 32,        // 已经是好友
    ADDFRIEND_REQUEST = 33, // 添加好友请求
    // 好友聊天回复
    STARTCHAT_SUCCESS = 35, // 成功开始聊天
    STARTCHAT_DENY = 36,    // 好友拒绝聊天请求
    ISCHATING = 37,         // 正在聊天，聊天窗口未关闭
    STARTCHAT_REQUEST = 38, // 开始聊天请求
    // 好友连接回复
    CONNECT_SUCCESS = 40,   // 连接成功
    CONNECT_FAILED = 41,    // 连接失败
    CONNECT_REQUEST = 43,   // 好友连接请求
    // 群聊回复
    GROUPCHAT_REQUEST = 45, // 群聊请求
    // 登出回复
    LOGOUT_SUCCESS = 50,    // 登出成功
    LOGOUT_FAILED = 51,      // 登出失败
    // 会话消息回复
    NO_MESSAGE = 60,        // 无消息
    NAVE_MESSAGE = 61,      // 有消息
    HAVE_TALK_MESSAGE = 62  // 有会消息

};

/// 全局变量
const QRegExp ipRegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");       // 判断 IP 地址正则表达式
// QRegExp ipRegExp("\\b((?:(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d)\\.)""{3}(?:25[0-5]|2[0-4]\\d|[01]?\\d?\\d))\\b");    // 判断 IP 地址正则表达式
const QRegExp portRegExp("(/^[1-9]$|(^[1-9][0-9]$)|(^[1-9][0-9][0-9]$)|(^[1-9][0-9][0-9][0-9]$)|(^[1-6][0-5][0-5][0-3][0-5]$)/)");    // 判断端口号正则表达式
const QRegExp macRegExp("([0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2}-[0-9A-F]{2})");    // 判断 MAC 地址正则表达式
const QRegExp accountRegExp("[0-9]{10}");   // 账号匹配正则表达式

/// 结构体定义
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
    QString account;    // 用户登录账号
    QString password;   // 用户登录密码
    QString nickname;   // 昵称
    QString avatar;     // 头像路径
    int status;         // 状态
    int request;        // 请求
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
    ServerNode node;        // 好友IP地址节点
    QTcpSocket *tcpSocket;    // 每个好友与自身服务器建立的TCP socket
    bool isConnected;       // 与该好友已经建立 TCP 通信
    QString name;           // 好友名字
    QString avatar;       // 好友头像路径（相对路径）
    int status;             // 好友在线状态
    QString block;          // 好友所在分组
    QString group;          // 好友所在的群

    QString about;          // 好友信息
    int friendKind;        //好友类型
    QString remark;			//好友备注

    FriendInfo()
    {	avatar = status = friendKind = 0;
        isConnected = false;
    }

    friend QDataStream & operator<< (QDataStream &qos, const FriendInfo &fi)
    {
        qos << fi.account << fi.name << fi.avatar << fi.node.hostAddr << fi.status
            << fi.about << fi.friendKind << fi.remark;
        return qos;
    }
    friend QDebug operator << (QDebug qd, const FriendInfo &fi) // 重载 qDebug() 数据流
    {
        qd << fi.account << fi.name << fi.avatar << fi.node.hostAddr << fi.status
            << fi.about << fi.friendKind << fi.remark;
        return qd;
    }

    friend QDataStream & operator>> (QDataStream &qis, FriendInfo &fi)
    {
        qis >> fi.account >> fi.name >> fi.avatar >> fi.status >> fi.node.hostAddr
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

//传送文件类
struct TSFile
{
    QFile *File;                //传送文件
    qint64 WholeBytes;          //数据总大小
    qint64 FinishedBytes;       //传送完成数据大小
    qint64 TodoBytes;           //待传送数据大小
    qint64 EachSize;            //每次传送数据大小
    QString FileName;           //文件名
    QByteArray Buffer;          //数据缓冲区

    TSFile()
    {
        WholeBytes = 0;
        FinishedBytes = 0;
        TodoBytes = 0;
        EachSize = 4*1024;
    }
};

//语音类
struct Speech
{
    QTcpServer      *SpeechServer;              //监听语音请求
    QTcpSocket      *SpeechSocket;              //语音传输Socket
    QAudioInput     *audio_in;                  //Input
    QAudioOutput    *audio_out;                 //Output
    QIODevice       *buffer_in;                 //输入设备
    QIODevice       *buffer_out;                //输出设备
    QAudioFormat     ad_format;                 //格式
    QByteArray      *SpeechBuffer_in;           //语音输入缓存区
    QByteArray      *SpeechBuffer_out;          //语音输出缓存区
    int             SpeechConnected;            //语音连接(0无连接1连接请求2已连接)
};

// 从账号中获取端口号
#ifndef GET_PORTNUMBER
#define GET_PORTNUMBER
inline quint16 getPortNumber(QString account)
{
    return (account.right(4).toUInt());     // 截取后4位作为服务器端口号
}
#endif // GET_PORTNUMBER

#endif // CLASSCONSTANT_H
