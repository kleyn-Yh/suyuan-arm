#include "httpserver.h"
#include "appconfig.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include<QNetworkAccessManager>
#include<QNetworkRequest>

HttpServer::HttpServer(QObject *parent)
    : HttpRequestHandler{parent}
{
    QString configFileName=":/new/http/webapp.ini";     //已经创建成功的文件
    QSettings* listenerSettings=new QSettings(configFileName, QSettings::IniFormat, this);
    listenerSettings->beginGroup("listener");	//新增代码

    new stefanfrings::HttpListener(listenerSettings, this, this);	//新增代码
}

void HttpServer::service(stefanfrings::HttpRequest &request, stefanfrings::HttpResponse &response)
{
    bool config_flag = false;       //传参是否正确
    QByteArray path = request.getPath();        //获取客户端或者网页端请求的URL
    QByteArray body= request.getBody();         //获取body参数,正文
    QByteArray method = request.getMethod();    //获取请求方法 GET\POST
    QByteArray param = request.getParameter("file_path"); //取参方式
//    myWarning() << "path: " << path <<endl;
//    myWarning() << "body: " << body <<endl;
//    myWarning() << "method: " << method <<endl;
//    myWarning() << "param: " << param <<endl;
    if(method.contains("PUT"))      //修改参数
    {
        if(path.contains("/alarm_config")){
            config_flag = put_alarm_config(body);
            emit pram_syncToMCU_signal();
        }
        else if(path.contains("/NVTconfig")){
            config_flag = put_NVTconfig(body);
        }
        else if(path.contains("/realtime_chan")){
            config_flag = put_realtime_chan(body);
        }
        else if(path.contains("/time_sync_with_nvt")){  //将球机时间同步置定位系统
            emit time_sync_signal();
            config_flag = true;
        }
        else if(path.contains("/basic")){               //修改设备名称
            config_flag = put_basic(body);
        }
        else if(path.contains("/hb_settings")){         //修改心跳发送设置
            config_flag = put_hb_settings(body);
        }
        else if(path.contains("/tcpsend_settings")){         //修改TCP服务器发送设置
            config_flag = put_tcpsend_settings(body);
        }
        else if(path.contains("/test_microphone")){         //通道麦克风测试
            config_flag = put_test_microphone(body);
        }
        else if(path.contains("/reboot")){              //重启设备
            QProcess::execute(QString("reboot"));
        }
        else if(path.contains("/system_time")){         //设置系统时间
            config_flag = put_system_time(body);
        }
        else if(path.contains("/directional_calibration")){  //进行方向校准
            AppConfig::is_calibration = true;
            config_flag = true;
        }
        else if(path.contains("/zero_calibration")){    //进行零点校准
            emit SetHomePosition_signal();
            config_flag = true;
        }
        else if(path.contains("/system_upgrade")){      //进行系统升级
            AppConfig::is_update_code = true;
            config_flag = true;
        }
        else if(path.contains("/self_ctrl_cam")){      //手动控制球机
            config_flag = put_ctrl_cam(body);
        }
        else if(path.contains("/set_dev_ip")){
            config_flag = put_set_ip(body); 
        }

        if(config_flag){
            response.write("HTTP OK");
        }else{
            response.setStatus(404,"HTTP BADREQUEST");
            response.write("HTTP BADREQUEST");
        }
    }
    else if(method.contains("GET"))
    {
        if(path.contains("/system_parm")){                  //获取运行参数接口
            get_system_parm(response);
        }
        else if(path.contains("/get_device_basic_params")){ //获取当前系统信息
            get_devInfo(response);
        }
        else if(path.contains("/hb_settings")){             //获取心跳发送设置
            get_hb_settings(response);
        }
        else if(path.contains("/tcpsend_settings")){
            get_tcpsend_settings(response);
        }
        else if(path.contains("/system_time")){             //获取系统时间
            get_system_time(response);
        }
        else if(path.contains("/alarm_file")){       //按条件保存日志
            get_alarm_file(response, param);
        }
        else if(path.contains("/test_result")){         //确认测试结果
            get_test_result(response);
        }
    }
    else if(method.contains("POST"))
    {
        if(path.contains("/hb_settings"))                   //平台主动获取心跳
        {
            post_heartInfo(response);
        }
        else if(path.contains("/get_all_alarms"))           //获取日志信息
        {
            post_alarmLogs(response, body, false);
        }
        else if(path.contains("/get_alarm_logs_conditional"))//按条件搜索日志
        {
            post_alarmLogs(response, body, true);
        }
    }
}
bool HttpServer::put_alarm_config(QByteArray json)
{
    QSqlQuery query(parmdb);
    // 将 JSON 数据转换为 QJsonDocument 对象
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning() << "不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象
    mutex.lock();
    //参数保存
    if(obj.contains ("notify_url")){
        AppConfig::AlarmParm.notify_url = obj["notify_url"].toString();
    }
    if(obj.contains ("is_send_alarm")){
        AppConfig::AlarmParm.is_send_alarm = obj["is_send_alarm"].toBool();
    }
    if(obj.contains ("is_send_realtime_value")){
        AppConfig::AlarmParm.is_send_realtime_value = obj["is_send_realtime_value"].toBool();
    }
    if(obj.contains ("db_limit_day")){
        AppConfig::AlarmParm.db_limit_day = obj["db_limit_day"].toDouble();
    }
    if(obj.contains ("db_limit_night")){
        AppConfig::AlarmParm.db_limit_night = obj["db_limit_night"].toDouble();
    }
    if(obj.contains ("tde_ratio_limit")){
        AppConfig::AlarmParm.tde_ratio_limit = obj["tde_ratio_limit"].toDouble();
    }
    if(obj.contains ("fde_ratio_limit")){
        AppConfig::AlarmParm.fde_ratio_limit = obj["fde_ratio_limit"].toDouble();
    }
    if(obj.contains ("freq_up_limit")){
        AppConfig::AlarmParm.freq_up_limit = obj["freq_up_limit"].toInt();
    }
    if(obj.contains ("freq_down_limit")){
        AppConfig::AlarmParm.freq_down_limit = obj["freq_down_limit"].toInt();
    }
    query.prepare("update ALARM_CONFIG set notify_url = :notify_url,is_send_alarm = :is_send_alarm,is_send_realtime_value = :is_send_realtime_value,db_limit_day = :db_limit_day,db_limit_night = :db_limit_night,tde_ratio_limit = :tde_ratio_limit,"
                  "fde_ratio_limit = :fde_ratio_limit,freq_up_limit = :freq_up_limit,freq_down_limit = :freq_down_limit  where id = :id");
    query.bindValue(":notify_url", AppConfig::AlarmParm.notify_url);
    query.bindValue(":is_send_alarm", AppConfig::AlarmParm.is_send_alarm);
    query.bindValue(":is_send_realtime_value", AppConfig::AlarmParm.is_send_realtime_value);
    query.bindValue(":db_limit_day", AppConfig::AlarmParm.db_limit_day);
    query.bindValue(":db_limit_night", AppConfig::AlarmParm.db_limit_night);
    query.bindValue(":tde_ratio_limit", AppConfig::AlarmParm.tde_ratio_limit);
    query.bindValue(":fde_ratio_limit", AppConfig::AlarmParm.fde_ratio_limit);
    query.bindValue(":freq_up_limit", AppConfig::AlarmParm.freq_up_limit);
    query.bindValue(":freq_down_limit", AppConfig::AlarmParm.freq_down_limit);
    query.bindValue(":id", 1);
    query.exec();

    mutex.unlock();
    return true;
}

bool HttpServer::put_NVTconfig(QByteArray json)
{
    QSqlQuery query(parmdb);
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象
    mutex.lock();
    //参数保存
    if(obj.contains ("cam_enable")){
        AppConfig::CamParm.cam_enable = obj["cam_enable"].toBool();
    }
    if(obj.contains ("cam_ip")){
        AppConfig::CamParm.cam_ip = obj["cam_ip"].toString();
    }
    if(obj.contains ("cam_pass")){
        AppConfig::CamParm.cam_pass = obj["cam_pass"].toString();
    }
    if(obj.contains ("cam_port")){
        AppConfig::CamParm.cam_port = obj["cam_port"].toInt();
    }
    if(obj.contains ("cam_user")){
        AppConfig::CamParm.cam_user = obj["cam_user"].toString();
    }
    if(obj.contains ("is_ptz_direction_same_with_sonar")){
        AppConfig::CamParm.is_ptz_direction_same_with_sonar = obj["is_ptz_direction_same_with_sonar"].toBool();
    }
    if(obj.contains ("is_save_snapshot")){
        AppConfig::CamParm.is_save_snapshot = obj["is_save_snapshot"].toBool();
    }
    query.prepare("update CAM_CONFIG set cam_enable = :cam_enable,cam_ip = :cam_ip,cam_pass = :cam_pass,cam_port = :cam_port,cam_user = :cam_user,"
                  "is_ptz_direction_same_with_sonar = :is_ptz_direction_same_with_sonar,is_save_snapshot = :is_save_snapshot  where id = :id");
    query.bindValue(":cam_enable", AppConfig::CamParm.cam_enable);
    query.bindValue(":cam_ip", AppConfig::CamParm.cam_ip);
    query.bindValue(":cam_pass", AppConfig::CamParm.cam_pass);
    query.bindValue(":cam_port", AppConfig::CamParm.cam_port);
    query.bindValue(":cam_user", AppConfig::CamParm.cam_user);
    query.bindValue(":is_ptz_direction_same_with_sonar", AppConfig::CamParm.is_ptz_direction_same_with_sonar);
    query.bindValue(":is_save_snapshot", AppConfig::CamParm.is_save_snapshot);
    query.bindValue(":id", 1);
    query.exec();

    mutex.unlock();
    return true;
}

bool HttpServer::put_realtime_chan(QByteArray json)
{
    QSqlQuery query(parmdb);
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象
    mutex.lock();
    //参数保存
    if(obj.contains ("cur_realtime_channel"))
    {
        AppConfig::CamParm.cur_realtime_channel = obj["cur_realtime_channel"].toInt();
    }
    query.prepare("update CAM_CONFIG set cur_realtime_channel = :cur_realtime_channel where id = :id");
    query.bindValue(":cur_realtime_channel", AppConfig::CamParm.cur_realtime_channel);
    query.bindValue(":id", 1);
    query.exec();

    mutex.unlock();
    return true;
}

bool HttpServer::put_basic(QByteArray json)
{
    QSqlQuery query(parmdb);
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象
    mutex.lock();
    //参数保存
    if(obj.contains ("dev_name"))
    {
        AppConfig::DevInfo.dev_name = obj["dev_name"].toString();
    }
    query.prepare("update DEV_INFO set dev_name = :dev_name where id = :id");
    query.bindValue(":dev_name", AppConfig::DevInfo.dev_name);
    query.bindValue(":id", 1);
    query.exec();

    mutex.unlock();
    return true;
}

bool HttpServer::put_hb_settings(QByteArray json)
{
    QSqlQuery query(parmdb);
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象
    mutex.lock();
    //参数保存
    if(obj.contains ("is_notify_heartbeat"))
    {
        AppConfig::DevHeart.is_notify_heartbeat = obj["is_notify_heartbeat"].toBool();
    }
    if(obj.contains ("notify_interval"))
    {
        AppConfig::DevHeart.notify_interval = obj["notify_interval"].toInt();
    }
    if(obj.contains ("notify_url"))
    {
        AppConfig::DevHeart.notify_url = obj["notify_url"].toString();
    }
    query.prepare("update DEV_HEART set is_notify_heartbeat = :is_notify_heartbeat,notify_interval = :notify_interval,notify_url = :notify_url where id = :id");
    query.bindValue(":is_notify_heartbeat", AppConfig::DevHeart.is_notify_heartbeat);
    query.bindValue(":notify_interval", AppConfig::DevHeart.notify_interval);
    query.bindValue(":notify_url", AppConfig::DevHeart.notify_url);
    query.bindValue(":id", 1);
    query.exec();

    mutex.unlock();
    return true;
}

bool HttpServer::put_tcpsend_settings(QByteArray json)
{
    QSqlQuery query(parmdb);
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象
    mutex.lock();
    //参数保存
    if(obj.contains ("is_send_tcpserver"))
    {
        AppConfig::DevTcpSend.is_send_tcpserver = obj["is_send_tcpserver"].toBool();
    }
    if(obj.contains ("tcpserver_addr"))
    {
        AppConfig::DevTcpSend.tcpserver_addr = obj["tcpserver_addr"].toString();
    }
    if(obj.contains ("tcpserver_port"))
    {
        AppConfig::DevTcpSend.tcpserver_port = obj["tcpserver_port"].toInt();
    }
    if(obj.contains ("type"))
    {
        AppConfig::DevTcpSend.type = obj["type"].toInt();
    }
    query.prepare("update DEV_TCPSEND set is_send_tcpserver = :is_send_tcpserver,tcpserver_addr = :tcpserver_addr,tcpserver_port = :tcpserver_port,type = :type where id = :id");
    query.bindValue(":is_send_tcpserver", AppConfig::DevTcpSend.is_send_tcpserver);
    query.bindValue(":tcpserver_addr", AppConfig::DevTcpSend.tcpserver_addr);
    query.bindValue(":tcpserver_port", AppConfig::DevTcpSend.tcpserver_port);
    query.bindValue(":type", AppConfig::DevTcpSend.type);
    query.bindValue(":id", 1);
    query.exec();

    mutex.unlock();

    if(AppConfig::DevTcpSend.type == 0x01)
    {
        emit tcpclientconnect_signal();
    }
    return true;
}

bool HttpServer::put_test_microphone(QByteArray json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象

    //参数保存
    if(obj.contains ("is_test_mic"))
    {
        AppConfig::is_test_mic = obj["is_test_mic"].toBool();
    }

    return true;
}

bool HttpServer::put_system_time(QByteArray json)
{
    QStringList arg;
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象
    if(obj.contains ("system_time"))
    {
        QString system_time = obj["system_time"].toString();
        //soft
        arg << "-s" << system_time;
        QProcess::execute("date", arg);

        arg.clear();
        arg << "-w";
        QProcess::execute("hwclock", arg);

        arg.clear();
        arg << "-w" << "-f" << "/dev/rtc0";
        QProcess::execute("hwclock", arg);
        return true;
    }
    return false;
}

bool HttpServer::put_ctrl_cam(QByteArray json)
{
    QSqlQuery query(parmdb);
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象

    if(obj.contains ("is_ctrl_cam"))
    {
        AppConfig::SelfCtrl.is_ctrl_cam = obj["is_ctrl_cam"].toBool();
    }
    if(obj.contains ("rotation_angle"))
    {
        AppConfig::SelfCtrl.rotation_angle = obj["rotation_angle"].toInt();
    }
    if(obj.contains ("angle_of_pitch"))
    {
        AppConfig::SelfCtrl.angle_of_pitch = obj["angle_of_pitch"].toInt();
    }
    if(AppConfig::SelfCtrl.is_ctrl_cam == true)
    {
        if(AppConfig::SelfCtrl.rotation_angle > 180)
        {
            AppConfig::SelfCtrl.rotation_angle = AppConfig::SelfCtrl.rotation_angle/180.0 - 2.0;
        }
        else
        {
            AppConfig::SelfCtrl.rotation_angle = AppConfig::SelfCtrl.rotation_angle/180.0;
        }
        AppConfig::CamParm.angle_of_pitch_ptz = AppConfig::SelfCtrl.angle_of_pitch/60.0 - 0.5;
        mutex.lock();
        query.prepare("update CAM_CONFIG set angle_of_pitch_ptz = :angle_of_pitch_ptz where id = :id");
        query.bindValue(":angle_of_pitch_ptz", AppConfig::CamParm.angle_of_pitch_ptz);
        query.bindValue(":id", 1);
        query.exec();
        mutex.unlock();
        emit ptzControl_signal();
    }
    return true;
}

bool HttpServer::put_set_ip(QByteArray json)
{
    QSqlQuery query(parmdb);
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if(!doc.isObject())
    {
        myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
        return false;
    }
    QJsonObject obj = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象
    mutex.lock();
    //参数保存
    if(obj.contains ("ip_address"))
    {
        AppConfig::DevInfo.dev_ip = obj["ip_address"].toString();
    }
    if(obj.contains ("ip_netmsk"))
    {
        AppConfig::DevInfo.dev_netmask = obj["ip_netmsk"].toString();
    }
    if(obj.contains ("ip_gateway"))
    {
        AppConfig::DevInfo.dev_gateway = obj["ip_gateway"].toString();
    }

    if((AppConfig::DevInfo.dev_ip == NULL)||(AppConfig::DevInfo.dev_netmask == NULL)||(AppConfig::DevInfo.dev_gateway == NULL))
    {
        return false;
    }
    query.prepare("update DEV_INFO set dev_ip = :dev_ip,dev_netmask = :dev_netmask,dev_gateway = :dev_gateway where id = :id");
    query.bindValue(":dev_ip", AppConfig::DevInfo.dev_ip);
    query.bindValue(":dev_netmask", AppConfig::DevInfo.dev_netmask);
    query.bindValue(":dev_gateway", AppConfig::DevInfo.dev_gateway);
    query.bindValue(":id", 1);
    query.exec();
    mutex.unlock();

    QFile fileRead("/etc/rc.local");       //先读取文件信息，QIODevice::Text能将换行转为\n
    QString arryRead;
    if(fileRead.open(QIODevice::ReadOnly|QIODevice::Text)){
        arryRead= fileRead.readAll();
    }
    else
    {
        myWarning() << "文件打开失败";
    }
    fileRead.close();
    QStringList arryListWrite= arryRead.split("\n");        //存储每一段信息
    myWarning()<<"arryListWrite:";
    for(int i=0;i<arryListWrite.size();i++){
        myWarning()<<arryListWrite.at(i);
    }

    QFile fileWrite("/etc/rc.local");
    if(fileWrite.open(QIODevice::WriteOnly|QIODevice::Text)){        //文件流写文件
        QTextStream streamWrite(&fileWrite);      //通过文本流控制文件写操作

        for(int i=0;i<arryListWrite.size()-1;i++){      //这里到arryListWrite.size()-1是因为arryListWrite数组按照\n分 段时，最后一行尾部有个\n，所以数组最后一个值为空，需要将它去掉

            if(arryListWrite.at(i).contains("ifconfig eth1")){

                QString contentWrite= arryListWrite.at(i);

                //-----读者可自行替换内容
                QRegExp regexp("ifconfig eth1 (\\d+.+\\d+.+\\d+.\\d+) netmask (\\d+.+\\d+.+\\d+.+\\d+)");
                regexp.indexIn(contentWrite,0);
                //myWarning()<<regexp.cap(1);
                contentWrite.replace(regexp.cap(1),AppConfig::DevInfo.dev_ip);
                contentWrite.replace(regexp.cap(2),AppConfig::DevInfo.dev_netmask);
                //-----读者可自行替换内容

                streamWrite<<contentWrite<<"\n";
            }
            else if((arryListWrite.at(i).contains("route add default gw"))&&(arryListWrite.at(i).contains("eth1")))
            {
                QString contentWrite2= arryListWrite.at(i);

                //-----读者可自行替换内容
                QRegExp regexp2("route add default gw (\\d+.+\\d+.+\\d+.+\\d+) eth1");
                regexp2.indexIn(contentWrite2,0);
                //myWarning()<< "regexp2.cap(1)" << regexp2.cap(1);
                contentWrite2.replace(regexp2.cap(1),AppConfig::DevInfo.dev_gateway);
                //-----读者可自行替换内容

                streamWrite<<contentWrite2<<"\n";
            }else{
                streamWrite<<arryListWrite.at(i)<<"\n";
            }
        }
    }
    fileWrite .close();
    AppConfig::is_update_code = true;
    return true;
}

void HttpServer::get_system_parm(stefanfrings::HttpResponse &response)
{
    //Json对象数据组织
    QJsonObject obj;
    obj.insert("cam_enable",AppConfig::CamParm.cam_enable);
    obj.insert("cam_ip",AppConfig::CamParm.cam_ip);
    obj.insert("cam_pass",AppConfig::CamParm.cam_pass);
    obj.insert("cam_port",AppConfig::CamParm.cam_port);
    obj.insert("cam_user",AppConfig::CamParm.cam_user);
    obj.insert("cur_realtime_channel",AppConfig::CamParm.cur_realtime_channel);
    obj.insert("db_limit_day",AppConfig::AlarmParm.db_limit_day);
    obj.insert("db_limit_night",AppConfig::AlarmParm.db_limit_night);
    obj.insert("tde_ratio_limit",AppConfig::AlarmParm.tde_ratio_limit);
    obj.insert("fde_ratio_limit",AppConfig::AlarmParm.fde_ratio_limit);
    obj.insert("freq_up_limit",AppConfig::AlarmParm.freq_up_limit);
    obj.insert("freq_down_limit",AppConfig::AlarmParm.freq_down_limit);
    obj.insert("is_ptz_direction_same_with_sonar",AppConfig::CamParm.is_ptz_direction_same_with_sonar);
    obj.insert("is_save_snapshot",AppConfig::CamParm.is_save_snapshot);
    obj.insert("is_send_alarm",AppConfig::AlarmParm.is_send_alarm);
    obj.insert("is_send_realtime_value",AppConfig::AlarmParm.is_send_realtime_value);
    obj.insert("notify_url",AppConfig::AlarmParm.notify_url);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    response.setHeader("Content-Type", "application/json");
    response.write(json);
}

void HttpServer::get_devInfo(stefanfrings::HttpResponse &response)
{
    //Json对象数据组织
    QJsonObject obj;
    obj.insert("dev_name",AppConfig::DevInfo.dev_name);
    obj.insert("dev_type",AppConfig::DevInfo.dev_type);
    obj.insert("dev_version",AppConfig::DevInfo.dev_version);
    obj.insert("equip_number",AppConfig::DevInfo.equip_number);
    obj.insert("upgrade_time",AppConfig::DevInfo.upgrade_time);
    obj.insert("web_version",AppConfig::DevInfo.web_version);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    response.setHeader("Content-Type", "application/json");
    response.write(json);
}

void HttpServer::get_hb_settings(stefanfrings::HttpResponse &response)
{
    //Json对象数据组织
    QJsonObject obj;
    obj.insert("is_notify_heartbeat",AppConfig::DevHeart.is_notify_heartbeat);
    obj.insert("notify_interval",AppConfig::DevHeart.notify_interval);
    obj.insert("notify_url",AppConfig::DevHeart.notify_url);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    response.setHeader("Content-Type", "application/json");
    response.write(json);
}

void HttpServer::get_tcpsend_settings(stefanfrings::HttpResponse &response)
{
    //Json对象数据组织
    QJsonObject obj;
    obj.insert("is_send_tcpserver",AppConfig::DevTcpSend.is_send_tcpserver);
    obj.insert("tcpserver_addr",AppConfig::DevTcpSend.tcpserver_addr);
    obj.insert("tcpserver_port",AppConfig::DevTcpSend.tcpserver_port);
    obj.insert("type",AppConfig::DevTcpSend.type);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    response.setHeader("Content-Type", "application/json");
    response.write(json);
}

void HttpServer::get_system_time(stefanfrings::HttpResponse &response)
{
    QString sysTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    //Json对象数据组织
    QJsonObject obj;
    obj.insert("system_time",sysTime);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    response.setHeader("Content-Type", "application/json");
    response.write(json);
}

void HttpServer::get_alarm_file(stefanfrings::HttpResponse &response, QByteArray param)
{
    QString param_path = param;
    QString file_path = QString("/home/baolong/Logs/%1").arg(param_path);
    if(param_path == NULL)
    {
        response.setStatus(404,"HTTP BADREQUEST");
        response.write("HTTP BADREQUEST");
        return;
    }
    //上传文件
    QFile file(file_path);
    if (!file.exists()) {
        response.setStatus(404,"HTTP NOTFOUND");
        response.write("HTTP NOTFOUND");
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        response.setStatus(404,"HTTP NOTFOUND");
        response.write("HTTP NOTFOUND");
        return;
    }
    QByteArray content = file.readAll();
    file.close();
    response.setHeader("Content-Type", "application/octet-stream");
    response.setHeader("Content-Length", QByteArray::number(content.size()));
    response.write(content);
}

void HttpServer::get_test_result(stefanfrings::HttpResponse &response)
{
    bool is_succeed = true;
    QString str = "";
    AppConfig::is_test_mic = false;
    for(int i=0; i<16; i++)
    {
        if(AppConfig::test_result[i] == 0)
        {
            is_succeed = false;
            if (!str.isEmpty()) {
                str.append(",");  // 添加逗号作为分隔符
            }
            str.append(QString::number(i+1));  // 将数字转换为字符串并追加
        }
        AppConfig::test_result[i] = 0;
    }
    //Json对象数据组织
    QJsonObject obj;
    obj.insert("is_succeed",is_succeed);
    obj.insert("fail_channal",str);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    response.setHeader("Content-Type", "application/json");
    response.write(json);
}

void HttpServer::post_heartInfo(stefanfrings::HttpResponse &response)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString currentTime = dateTime.toString("yyyy-MM-dd_hh-mm-ss");
    int nTimeStep = dateTime.toSecsSinceEpoch();
    //Json对象数据组织
    QJsonObject obj;
    obj.insert("message_type","heart_beat_message");
    obj.insert("sys_time",nTimeStep);
    obj.insert("sys_time_str",currentTime);
    obj.insert("dev_name",AppConfig::DevInfo.dev_name);
    obj.insert("dev_type",AppConfig::DevInfo.dev_type);
    obj.insert("dev_version",AppConfig::DevInfo.dev_version);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    response.setHeader("Content-Type", "application/json");
    response.write(json);
}

void HttpServer::post_alarmLogs(stefanfrings::HttpResponse &response, QByteArray json, bool conditional)
{
    int cur_logs_count = 0;
    QSqlQuery query_logs(logdb);//数据库的关联
    //Json对象数据组织
    QJsonObject obj;
    QJsonArray array;

    if(conditional == true)     //按照条件搜索
    {
        QJsonDocument doc = QJsonDocument::fromJson(json);
        int s_nTimeStep;
        int e_nTimeStep;

        if(!doc.isObject())
        {
            response.setStatus(404,"HTTP BADREQUEST");
            response.write("HTTP BADREQUEST");
            myWarning()<<"不是 JSON 对象：JSON 字符串格式错误或无效";
            return ;
        }
        QJsonObject obj_g = doc.object();     // 将 QJsonDocument 对象中的 JSON 数据转换为 JSON 对象
        if(obj_g.contains ("start_time"))
        {
            QString stringtime = obj_g["start_time"].toString();
            QLocale locale(QLocale::English); // 使用英语语言环境
            QDateTime startTime = locale.toDateTime(stringtime, "yyyyMMddHHmm");
            s_nTimeStep = startTime.toSecsSinceEpoch();
        }
        if(obj_g.contains ("end_time"))
        {
            QString stringtime = obj_g["end_time"].toString();
            QLocale locale1(QLocale::English);
            QDateTime endTime = locale1.toDateTime(stringtime, "yyyyMMddHHmm");
            e_nTimeStep = endTime.toSecsSinceEpoch();
        }
        QString select_all_sql = QString("select * from ALARM_LOGS where nTimeStep > '%1' and nTimeStep < '%2'").arg(s_nTimeStep).arg(e_nTimeStep);
        query_logs.prepare(select_all_sql);

        if(!query_logs.exec(select_all_sql))
        {
            myWarning()<<query_logs.lastError();
        }
        else
        {
            cur_logs_count = 0;
            while(query_logs.next()){
                QJsonObject arr_obj;
                arr_obj.insert("id", query_logs.value(0).toInt());
                arr_obj.insert("time", query_logs.value(2).toString());
                arr_obj.insert("db_value", query_logs.value(3).toDouble());
                arr_obj.insert("tde_ratio", query_logs.value(4).toDouble());
                arr_obj.insert("fde_ratio", query_logs.value(5).toDouble());
                arr_obj.insert("horizontal", query_logs.value(6).toInt());
                arr_obj.insert("vertical", query_logs.value(7).toInt());
                arr_obj.insert("saving_path", query_logs.value(8).toString());
                arr_obj.insert("files_status", query_logs.value(9).toBool());
                array.append(arr_obj);
                cur_logs_count++;
            }
        }
    }
    else
    {
        QString select_all_sql ="select * from ALARM_LOGS ";
        query_logs.prepare(select_all_sql);
        if(!query_logs.exec(select_all_sql))
        {
            myWarning()<<query_logs.lastError();
        }
        else
        {
            cur_logs_count = 0;
            while(query_logs.next()){
                QJsonObject arr_obj;
                arr_obj.insert("id", query_logs.value(0).toInt());
                arr_obj.insert("time", query_logs.value(2).toString());
                arr_obj.insert("db_value", query_logs.value(3).toDouble());
                arr_obj.insert("tde_ratio", query_logs.value(4).toDouble());
                arr_obj.insert("fde_ratio", query_logs.value(5).toDouble());
                arr_obj.insert("horizontal", query_logs.value(6).toInt());
                arr_obj.insert("vertical", query_logs.value(7).toInt());
                arr_obj.insert("saving_path", query_logs.value(8).toString());
                arr_obj.insert("files_status", query_logs.value(9).toBool());
                array.append(arr_obj);
                cur_logs_count++;
            }
        }
    }
    obj.insert("logs",array);
    obj.insert("logs_count",cur_logs_count);
    QJsonDocument doc(obj);
    QByteArray s_json = doc.toJson();//json数据转文本格式字符串
    response.setHeader("Content-Type", "application/json");
    response.write(s_json);
}











