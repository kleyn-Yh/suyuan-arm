#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent): QObject(parent)
{
    tcpSocket = new QTcpSocket(this);
    reconnectTimer = new QTimer(this);

    connect(tcpSocket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    //connect(tcpSocket, &QTcpSocket::errorOccurred, this, &TcpClient::onErrorOccurred);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);

    connect(reconnectTimer, &QTimer::timeout, this, &TcpClient::attemptReconnect);
}

TcpClient::~TcpClient()
{
    tcpSocket->disconnectFromHost();
}
/*
 * @功	能	连接服务器
 * @参	数	无
 * */
void TcpClient::connectToServer()
{
    if (tcpSocket->state() == QAbstractSocket::UnconnectedState) {
        tcpSocket->connectToHost(AppConfig::DevTcpSend.tcpserver_addr, AppConfig::DevTcpSend.tcpserver_port);
    }
}
/*
 * @功	能	断开连接服务器
 * @参	数	无
 * */
void TcpClient::disconnectToServer()
{
    tcpSocket->disconnectFromHost();
    reconnectTimer->stop(); // 停止重连定时器
}
/*
 * @功	能	发送数据到服务器
 * @参	数	message
 * */
void TcpClient::sendMessage(const QString &message)
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->write(message.toUtf8());
    } else {
        emit errorOccurred("Not connected to the server.");
        myWarning() << "Not connected to the server.";
    }
}
/*
 * @功	能	停止重连到服务器
 * @参	数	无
 * */
void TcpClient::onConnected()
{
    myWarning() << "Connected to server:" << AppConfig::DevTcpSend.tcpserver_addr << AppConfig::DevTcpSend.tcpserver_port;
    AppConfig::TcpClientNetError = false;
    emit connected();
    reconnectTimer->stop(); // 停止重连定时器
}
/*
 * @功	能
 * @参	数	无
 * */
void TcpClient::onReadyRead()
{
    QByteArray data = tcpSocket->readAll();
    myWarning() << "Received data:" << data;
    // 解析或处理数据
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError); // 标记参数未使用
    emit errorOccurred(tcpSocket->errorString());
}
/*
 * @功	能	断开连接的回调函数
 * @参	数	无
 * */
void TcpClient::onDisconnected()
{
    myWarning() << "Disconnected from server.";
    AppConfig::TcpClientNetError = true;
    emit disconnected();
    reconnectTimer->start(30000); // 每30秒尝试重连
}
/*
 * @功	能	重连定时器触发
 * @参	数	无
 * */
void TcpClient::attemptReconnect()
{
    myWarning() << "Attempting to reconnect...";
    connectToServer();
}
