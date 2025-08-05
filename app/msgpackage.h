#ifndef MSGPACKAGE_H
#define MSGPACKAGE_H

#include <QObject>
#include <QByteArray>
#include "crcchecksum.h"

#define MAX_DATA_LEN 1024 * 100       //数据长度阈值为100K

class msgpackage : public QObject
{
    Q_OBJECT
public:
    msgpackage();

    // 重载构造函数，可控制是否开启校验和
    msgpackage(bool checksum);

    // 获取固定的协议包头长度
    int headerLength();

    // 清理包头和包内容数据
    void clearPkg();

    // 拆包函数
    void unPkg(QByteArray data);

    // 组包函数
    QByteArray mkPkg(QByteArray data);

    // 组成完整的包后调用该函数
    void pkgReady(QByteArray data);

signals:
    // 自定义信号，用于通知调用者有完整数据包，调用者需要使用connect绑定到自己的槽函数
    void sigPkgReady(QByteArray data);


private:

    // 内部函数，查找包头的位置，未找到返回-1
    int seekHeader(QByteArray data, int from = 0);

    // 内部函数，int转QByteArray
    QByteArray intToBytes(int i);

    // 内部函数，QByteArray转int
    quint32 bytesToInt(QByteArray bytes);

    // 内部函数，计算校验和
    quint8 getCheckSum(QByteArray data);

    // 内部函数，检测包的状态
    int checkWhatToDo();
private:
    crcCheckSum crcCheck; // 创建crcCheckSum类的实例

    bool isCheckSum;            //是否验证校验和
    QByteArray sig;             //头标志
    quint32 len;                //数据长度
    quint8 checksum;            //校验和
    QByteArray pkgData;         //包数据内容
    QByteArray tmpPkgData;      //临时数据，用拼接上一次的包数据，进入下一次循环迭代处理

};

#endif // MSGPACKAGE_H
