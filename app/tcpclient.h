#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>

#include "appconfig.h"

class TcpClient : public QObject
{
    Q_OBJECT
public:
    TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    void connectToServer();
    void disconnectToServer();
    void sendMessage(const QString &message);

signals:
    void messageReceived(const QString &message);
    void errorOccurred(const QString &error);
    void connected();
    void disconnected();

private slots:
    void onConnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onDisconnected();
    void attemptReconnect();

private:
    QTcpSocket *tcpSocket;
    QTimer *reconnectTimer; // 用于重连的定时器
};

#endif // TCPCLIENT_H
