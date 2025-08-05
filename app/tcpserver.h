#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QtCore>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QObject>
#include <QString>
#include <QDebug>

#include "appconfig.h"
#include "msgpackage.h"

#define TCP_PORT    1234

#define AUDIO_NUM_1S    10              //1s接收音频的帧数

#define ONE_LENGTH      2560//512*5           //uint16_t型

#define u16_DATA_LENGTH   ONE_LENGTH*16           //每帧0.1s的总数据长度 uint16_t型

#define u8_DATA_LENGTH    u16_DATA_LENGTH*2          //每帧0.1s的总数据长度 uint8_t型


class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

    ~TcpServer();

    bool startServer(quint16 port);

    void Close(void);
public:
     msgpackage msgPackage;

signals:
    void receivedData(const QByteArray &data);

private slots:
    void newClientHandler();

    void tcpUnPkg();

    void clientDisConnected();
private:
    QTcpSocket *socket;



};

#endif // TCPSERVER_H
