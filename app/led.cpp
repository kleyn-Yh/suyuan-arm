#include "led.h"
#include "appconfig.h"

led::led(QWidget *parent) : QWidget(parent)
{
    file.setFileName("/sys/devices/platform/leds/leds/sys-led/brightness");

    if (!file.exists())
    {
        myWarning()<<"未获取到 LED 设备！";
    }

    getLedState();
}
/**
  * @功	能	setLedState
  * @参	数	no
  */
void led::setLedState()
{
    /* 在设置 LED 状态时先读取 */
    bool state = getLedState();

    /* 如果文件不存在，则返回 */
    if (!file.exists())
        return;
    if(!file.open(QIODevice::ReadWrite))
        myWarning()<<file.errorString();
    QByteArray buf[2] = {"0", "1"};
    if (state)
        file.write(buf[0]);
    else
        file.write(buf[1]);
    /* 关闭文件 */
    file.close();
    /*重新获取 LED 的状态 */
    //getLedState();
}
/**
  * @功	能	getLedState
  * @参	数	no
  */
bool led::getLedState()
{
    /* 如果文件不存在，则返回 */
    if (!file.exists())
        return false;
    if(!file.open(QIODevice::ReadWrite))
        myWarning()<<file.errorString();
    QTextStream in(&file);
    /* 读取文件所有数据 */
    QString buf = in.readLine();
    /* 打印出读出的值 */
    file.close();
    if (buf == "1") {
        return true;
    } else {
        return false;
    }
}
