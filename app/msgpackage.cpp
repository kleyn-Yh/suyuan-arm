// 文件名：msgpackage.cpp
// created by yanghe
// 2024/08/08
#include "msgpackage.h"

#include "crcchecksum.h"
#include <QString>
#include "appconfig.h"

msgpackage::msgpackage()
{
    // 预设头标志和长度
    sig.resize(4);
    sig[0] = 0xff;
    sig[1] = 0xfd;
    sig[2] = 0xfe;
    sig[3] = 0xff;
    sig[4] = 0x7b;
    sig[5] = 0x7b;
    len = 0;
    checksum = 0;
    isCheckSum = false;

}

msgpackage::msgpackage(bool c)
{
    // 预设头标志和长度
    sig.resize(4);
    sig[0] = 0xff;
    sig[1] = 0xfd;
    sig[2] = 0xfe;
    sig[3] = 0xff;
    sig[4] = 0x7b;
    sig[5] = 0x7b;
    len = 0;
    checksum = 0;
    isCheckSum = c;
}

// 返回包头长度
int msgpackage::headerLength()
{
    return sig.length() + sizeof (quint8) + sizeof (uint);
}

// 清空数据包
void msgpackage::clearPkg()
{
    pkgData.clear();
    tmpPkgData.clear();
    len = 0;
    checksum = 0;
}

// 封装数据包
QByteArray msgpackage::mkPkg(QByteArray data)
{
    // 首先将校验和置0
    quint8 ckSum = 0;
    // 计算数据长度
    quint32 dataLen = data.length();
    // 组包
    QByteArray resData;
    resData.append(sig);
    resData.append(ckSum);
    resData.append(intToBytes(dataLen));
    resData.append(data);
    // 重新计算校验和并写入对应位置
    ckSum = crcCheck.Crc8(resData);
    resData[4] = ckSum;
    return resData;
}

// 拆封数据包
void msgpackage::unPkg(QByteArray data)
{
    switch(checkWhatToDo())
    {
    case 1:     // 等待接收新的包
    {
        // 寻找包头标志位置
        int index = seekHeader(data);
        // 舍弃包头标志前面的脏数据
        if(index >= 0)
        {
            data = data.right(data.length() - index);
            // 如果包头标志后的数据长度大于包头长度，则下一步处理
            if(data.length() > headerLength())
            {
                // 获取包数据长度
                len = bytesToInt(data.mid(sig.length() + sizeof (quint8), sizeof (quint32)));
                // 获取包校验和
                checksum = data[sig.length()];
                //myWarning() << "接收到包长度："<< len << "校验和：" << checksum ;
                // 剩下的数据迭代自身，交给case3处理
                if(len > 0)
                {
                    data = data.right(data.length() - headerLength());
                    unPkg(data);
                }
                else
                {
                    clearPkg();
                }
            }
            // 否则将包头标志后的数据存入临时数据等待下一次接收数据后拼接处理
            else
            {
                tmpPkgData = data;
            }
        }
        // 如果找不到可能是包头数据不完整，存入临时数据等待下一次接收数据后拼接处理
        else
        {
            tmpPkgData = data;
        }
        break;
    }
    case 2:     // 等待接收剩余的包数据
    {
        QByteArray mergeData = tmpPkgData + data;
        clearPkg();
        // 这里要设立一个阈值，防止一直脏数据导致mergeData过大导致内存崩溃
        // 如果mergeData超过阈值MAX_DATA_LEN，则直接清空包并退出循环
        if(mergeData.length() < MAX_DATA_LEN)
        { unPkg(mergeData); }
        else
        { myWarning() << "【错误】脏数据超过阈值"; }
        break;
    }
    case 3:     // 等待接受剩余的包内容数据
    {
        // 包剩余未接收的数据长度
        int surplusLen = len - pkgData.length();
        // 如果剩余包内容数据长度大于当前数据长度，直接写入
        if(surplusLen > data.length())
        {
            pkgData.append(data);
        }
        // 如果小于等于当前数据长度，则先写入当前包的剩余数据并打包好后发送ready信号
        // 然后将剩余数据迭代自身进行进一步处理
        else
        {
            pkgData.append(data.left(surplusLen));
            pkgReady(pkgData);
            clearPkg();

            data = data.right(data.length() - surplusLen);
            unPkg(data);
        }
        break;
    }
    default:
    {
        myWarning() << "【错误】未知的封包情况";
    }
    }
}

// 拆出完整数据包后调用该函数
void msgpackage::pkgReady(QByteArray data)
{
    // 是否开启校验位
    if(isCheckSum)
    {
        // 验证校验和
        quint8 cksum = getCheckSum(data);
        if(cksum == checksum)
        {
            //myWarning() << "【完整数据包】大小： " << data.length() << " 内容： " << data.data();
            // 发送自定义信号给调用者的槽函数，通知调用者一次完整数据包已生成
            emit this->sigPkgReady(data);
        }
        else
        {
            myWarning() << "【错误数据包】包校验和错误" << cksum << checksum;
        }
    }
    else
    {
        //myWarning() << "【完整数据包】大小： " << data.length() << " 内容： " << data.data();
        emit this->sigPkgReady(data);
    }
}

// *************************** 以下为类私有函数 ***************************

// 将int数据转化为QByteArray类型
QByteArray msgpackage::intToBytes(int i)
{
    QByteArray abyte0;
    abyte0.resize(4);
    abyte0[0] = (uchar)  (0x000000ff & i);
    abyte0[1] = (uchar) ((0x0000ff00 & i) >> 8);
    abyte0[2] = (uchar) ((0x00ff0000 & i) >> 16);
    abyte0[3] = (uchar) ((0xff000000 & i) >> 24);
    return abyte0;
}

// 将QByteArray类型数据转化为int
quint32 msgpackage::bytesToInt(QByteArray bytes) {
    if(bytes.length() < 4)
        return 0;
    int addr = bytes[0] & 0x000000FF;
    addr |= ((bytes[1] << 8) & 0x0000FF00);
    addr |= ((bytes[2] << 16) & 0x00FF0000);
    addr |= ((bytes[3] << 24) & 0xFF000000);
    return addr;
}


// 在QByteArray数据中寻找包头的位置
int msgpackage::seekHeader(QByteArray data, int from)
{
    int index = data.indexOf(sig, from);
    //myWarning() << "包头位置：" <<index;
    return index;
}

// 计算数据的checksum
quint8 msgpackage::getCheckSum(QByteArray data)
{
    // 首先将校验和置0
    quint8 ckSum = 0;
    // 计算数据长度
    quint32 dataLen = data.length();
    // 组包
    QByteArray resData;
    resData.append(sig);
    resData.append(ckSum);
    resData.append(intToBytes(dataLen));
    resData.append(data);
    // 重新计算校验和
    ckSum = crcCheck.Crc8(resData);
    return ckSum;
}

// 检测目前包的状态
// 检测len,pkgData,checksum,tmpHeaderData状态，判断是应该接收包头开始的数据还是包剩余数据
int msgpackage::checkWhatToDo()
{
    if(len == 0 && tmpPkgData.isEmpty())
    { return 1; }       // 等待接收新的包
    else if(len == 0 && !tmpPkgData.isEmpty())
    { return 2; }       // 等待接收剩余的包数据
    else if(len > 0 && tmpPkgData.isEmpty())
    { return 3; }       // 等待接受剩余的包内容数据
    return 0;
}
