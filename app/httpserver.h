#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include "httpserver/httprequesthandler.h"
#include <QSettings>
#include "httpserver/httplistener.h"	//新增代码
#include "httpserver/httprequesthandler.h"	//新增代码
#include "httpserver/staticfilecontroller.h"
#include "httpserver.h"
#include <QFile>
#include <QFileInfo>
#include <QProcess>

class HttpServer  : public stefanfrings::HttpRequestHandler
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);

    void service(stefanfrings::HttpRequest& request, stefanfrings::HttpResponse& response);

     QMutex mutex;  //互斥锁

     /* 通讯相关函数 */
     bool put_alarm_config(QByteArray json);    //修改报警参数

     bool put_NVTconfig(QByteArray json);       //修改球机参数

     bool put_realtime_chan(QByteArray json);   //修改实时音频流回传通道编号

     bool put_basic(QByteArray json);           //修改设备名称

     bool put_hb_settings(QByteArray json);     //修改心跳发送设置

     bool put_tcpsend_settings(QByteArray json);//修改TCP服务器发送设置

     bool put_test_microphone(QByteArray json);//通道麦克风测试

     bool put_system_time(QByteArray json);     //设置系统时间

     bool put_ctrl_cam(QByteArray json);        //手动控制球机

     bool put_set_ip(QByteArray json);          //设置球机IP

     void get_system_parm(stefanfrings::HttpResponse &response);   //获取运行参数接口

     void get_devInfo(stefanfrings::HttpResponse &response);       //发送设备信息

     void get_hb_settings(stefanfrings::HttpResponse &response);       //发送设备信息

     void get_tcpsend_settings(stefanfrings::HttpResponse &response);

     void get_system_time(stefanfrings::HttpResponse &response);       //获取系统时间

     void get_alarm_file(stefanfrings::HttpResponse &response, QByteArray param);       //按条件保存日志

     void get_test_result(stefanfrings::HttpResponse &response); //查询测试结果

     void post_heartInfo(stefanfrings::HttpResponse &response);     //发送心跳信息

     void post_alarmLogs(stefanfrings::HttpResponse &response, QByteArray json, bool conditional);     //发送报警日志

signals:
     void pram_syncToMCU_signal();     //同步参数到ARM

     void SetHomePosition_signal();     //球机方向校准

     void time_sync_signal();           //同步球机时间

     void ptzControl_signal();           //控制球机的信号

     void tcpclientconnect_signal();           //tcp客户端连接信号
};

#endif // HTTPSERVER_H
