#ifndef CLASSCONSTANT_H
#define CLASSCONSTANT_H
// 类与结构体的定义
#include <QHostAddress>
// ServerNode 节点类
struct ServerNode{
    QHostAddress hostAddr;
    quint16 hostPort;
    ServerNode()
    {
        hostPort = 0; /*  */
    }
};

#endif // CLASSCONSTANT_H
