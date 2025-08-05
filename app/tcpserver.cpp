#include "tcpserver.h"
#include <QtGui>

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    socket = nullptr;
    connect(this, SIGNAL(newConnection()), this, SLOT(newClientHandler()));
}

TcpServer::~TcpServer()
{
    Close();
}

void TcpServer::Close(void)
{
    delete socket;
}

bool TcpServer::startServer(quint16 port)
{
    if(!this->listen(QHostAddress::AnyIPv4, port))
    {
        myWarning() << "TCP listen not bind:" << port;
        return false;
    }
    myInfo() << "TCPListener: Listening on port 1234" ;
    return true;
}

void TcpServer::newClientHandler()
{
    //建立TCP连接
    socket = this->nextPendingConnection();
    socket->peerAddress();
    socket->peerPort();
    myInfo() << "TCP client Address:" << socket->peerAddress().toString();
    myInfo() << "TCP client Port:" << QString::number(socket->peerPort());

    connect(socket, SIGNAL(readyRead()), this, SLOT(tcpUnPkg()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisConnected()));// 客户端断开连接
    AppConfig::TcpconnectError = false;

}

void TcpServer::tcpUnPkg()
{
    msgPackage.unPkg(socket->readAll());
}


void TcpServer::clientDisConnected()
{
    AppConfig::TcpconnectError = true;
    myWarning() << "TCP client disconnected!";
}

