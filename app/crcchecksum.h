#ifndef CRCCHECKSUM_H
#define CRCCHECKSUM_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QObject>

typedef unsigned char u8;
typedef unsigned char byte;
typedef unsigned short u16;
typedef unsigned int u32;

class crcCheckSum : public QObject
{
public:
    crcCheckSum();
    quint16 crc16ForModbus(const QByteArray &data);

    quint8 Crc8(const QByteArray &data);
    quint16 Crc16(const QByteArray &data);
    u32 Crc32(byte *_pBuff, u16 _size);
};

#endif // CRCCHECKSUM_H
