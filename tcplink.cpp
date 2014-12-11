#include "tcplink.h"

TCPLink::TCPLink(const ServerNode &node, QObject *parent/* = 0*/) :
    QObject(parent), serverNode(node)
{
}

TCPLink::~TCPLink()
{

}
