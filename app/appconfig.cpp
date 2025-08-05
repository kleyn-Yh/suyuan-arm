#include "appconfig.h"

#include <QDebug>

QSqlDatabase parmdb;
QSqlDatabase logdb;

QString AppConfig::SearchDeviceADDR = "";  //指定设备地址

int AppConfig::SearchInterval = 1000;

int AppConfig::SearchTimeout = 3000;

bool AppConfig::SearchClear = true;

bool AppConfig::NetworkError = true;

bool AppConfig::is_test_mic = false;

uint8_t AppConfig::test_result[] = {0};

bool AppConfig::TcpClientNetError = true;

bool AppConfig::ProfilesError = true;

bool AppConfig::TcpconnectError = true;

bool AppConfig::is_calibration = false;

bool AppConfig::is_update_code = false;

ALARM_PARMCONFIG AppConfig::AlarmParm = {
    .notify_url = "",
    .is_send_alarm = false,
    .is_send_realtime_value = false,
    .db_limit_day = 70.0,
    .db_limit_night = 60.0,
    .tde_ratio_limit = 1.0,
    .fde_ratio_limit = 0.1,
    .freq_up_limit = 10000,
    .freq_down_limit = 5000,
    .signal_b = 11600           //默认值11600
};
CAM_PARMCONFIG AppConfig::CamParm = {
    .cam_enable = true,
    .cam_ip = "10.10.10.22",
    .cam_pass = "AHBL2023",
    .cam_port = 22,
    .cam_user = "baolong",
    .cur_realtime_channel = 1,
    .is_ptz_direction_same_with_sonar = true,
    .is_save_snapshot = true,
    .angle_of_pitch_ptz = 0.5,
};
DEV_INFOCONFIG AppConfig::DevInfo = {
    .dev_name = "",
    .dev_ip = "",
    .dev_netmask = "",
    .dev_gateway = "",
    .dev_type = "SUYUAN",
    .dev_version = "V1.2.4",
    .upgrade_time = "2024_12_13",
    .web_version = "V1.0.1",
    .equip_number = "XXXXXXXX"
};
ALARM_LOGINFO AppConfig::AlarmInfo = {
    .id = 0,
    .nTimeStep = 0,
    .time = "",
    .db_value = 0,
    .tde_ratio = 0,
    .fde_ratio = 0,
    .horizontal = 0,
    .vertical = 0,
    .saving_path = "",
    .files_status = 0,
    .cur_logs_sumnum = 0,
    .cur_logs_number = 0
};

SELF_CTRL AppConfig::SelfCtrl = {
    .is_ctrl_cam = false,
    .rotation_angle = 0,
    .angle_of_pitch = 0,
};

DEV_HEART AppConfig::DevHeart = {
    .is_notify_heartbeat = false,
    .notify_interval = 0,
    .notify_url = "",
};

DEV_TCPSEND AppConfig::DevTcpSend = {
    .is_send_tcpserver = false,
    .tcpserver_addr = "",
    .tcpserver_port = 0,
    .type = 0
};

void mySqlDatebase_Init(void)
{
    QString filePath = QString("/home/baolong/Sqls/");
    QDir dir(filePath);
    if(!dir.exists()){
        bool ok = dir.mkpath(filePath);
        if(!ok){
            myWarning() << "SQL filePath error!" << endl;
            return;
        }
    }
    parmdb = QSqlDatabase::addDatabase("QSQLITE","myParam.db");
    parmdb.setDatabaseName("/home/baolong/Sqls/myParam.db");
    if(!parmdb.open())
    {
        myWarning() << "myParam.db open error!" << parmdb.lastError();
    }
    //访问数据库的操作主要包括：创建表 向数据库中插入数据、删除数据、更新数据、查询数据
    //对于数据库中的表,通常只需要创建一次,而其他的操作可以重复
    QSqlQuery query_parm(parmdb);//在创建该对象时，系统自动完成跟数据库的关联

    QString cmd_str = QString("SELECT name FROM sqlite_master WHERE type='table' AND name='ALARM_CONFIG'");
    //查询是否建立了表格，没有建立则建立
    if(!(query_parm.exec(cmd_str) && query_parm.next()))  //如果表格没有被创建
    {
        myWarning() << "ALARM_CONFIG table does not exist";
        //定义一条创建表的sql语句 表名: MYPARMCONFIG 表中的字段: id  name age
        QString sqlCreate = QString("create table ALARM_CONFIG(id INTEGER PRIMARY KEY,"
                                    "notify_url QString,is_send_alarm int,is_send_realtime_value int,db_limit_day float,db_limit_night float,"
                                    "tde_ratio_limit float,fde_ratio_limit float,freq_up_limit int,freq_down_limit int,signal_b int);");
        query_parm.exec(sqlCreate);
        QString insert = QString("insert into ALARM_CONFIG(notify_url,is_send_alarm,is_send_realtime_value,db_limit_day,db_limit_night,tde_ratio_limit,fde_ratio_limit,freq_up_limit,freq_down_limit,signal_b) "
                                 "values('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10')")
                .arg(AppConfig::AlarmParm.notify_url).arg(AppConfig::AlarmParm.is_send_alarm).arg(AppConfig::AlarmParm.is_send_realtime_value)
                .arg(AppConfig::AlarmParm.db_limit_day).arg(AppConfig::AlarmParm.db_limit_night).arg(AppConfig::AlarmParm.tde_ratio_limit).arg(AppConfig::AlarmParm.fde_ratio_limit)
                .arg(AppConfig::AlarmParm.freq_up_limit).arg(AppConfig::AlarmParm.freq_down_limit).arg(AppConfig::AlarmParm.signal_b);
        query_parm.exec(insert);
    }
    else
    {
        QString select_all_sql = "select * from ALARM_CONFIG";
                query_parm.prepare(select_all_sql);
        if(!query_parm.exec())
        {
            myWarning() << query_parm.lastError();
        }
        else
        {
            while(query_parm.next()){
                AppConfig::AlarmParm.notify_url = query_parm.value(1).toString();
                AppConfig::AlarmParm.is_send_alarm = query_parm.value(2).toBool();
                AppConfig::AlarmParm.is_send_realtime_value = query_parm.value(3).toBool();
                AppConfig::AlarmParm.db_limit_day = query_parm.value(4).toDouble();
                AppConfig::AlarmParm.db_limit_night = query_parm.value(5).toDouble();
                AppConfig::AlarmParm.tde_ratio_limit = query_parm.value(6).toDouble();
                AppConfig::AlarmParm.fde_ratio_limit = query_parm.value(7).toDouble();
                AppConfig::AlarmParm.freq_up_limit = query_parm.value(8).toInt();
                AppConfig::AlarmParm.freq_down_limit = query_parm.value(9).toInt();
                AppConfig::AlarmParm.signal_b = query_parm.value(10).toInt();
            }
        }
    }
    cmd_str = QString("SELECT name FROM sqlite_master WHERE type='table' AND name='CAM_CONFIG'");
    //查询是否建立了表格，没有建立则建立
    if(!(query_parm.exec(cmd_str) && query_parm.next()))  //如果表格没有被创建
    {
        myWarning() << "CAM_CONFIG table does not exist";
        //定义一条创建表的sql语句 表名: MYPARMCONFIG 表中的字段: id  name age
        QString sqlCreate = QString("create table CAM_CONFIG(id INTEGER PRIMARY KEY,"
                                    "cam_enable int,cam_ip QString,cam_pass QString,cam_port float,"
                                    "cam_user QString,cur_realtime_channel int,is_ptz_direction_same_with_sonar int,is_save_snapshot int,angle_of_pitch_ptz int);");
        query_parm.exec(sqlCreate);
        QString insert = QString("insert into CAM_CONFIG(cam_enable,cam_ip,cam_pass,cam_port,cam_user,cur_realtime_channel,is_ptz_direction_same_with_sonar,is_save_snapshot,angle_of_pitch_ptz) "
                                 "values('%1','%2','%3','%4','%5','%6','%7','%8','%9')")
                .arg(AppConfig::CamParm.cam_enable).arg(AppConfig::CamParm.cam_ip).arg(AppConfig::CamParm.cam_pass)
                .arg(AppConfig::CamParm.cam_port).arg(AppConfig::CamParm.cam_user).arg(AppConfig::CamParm.cur_realtime_channel)
                .arg(AppConfig::CamParm.is_ptz_direction_same_with_sonar).arg(AppConfig::CamParm.is_save_snapshot).arg(AppConfig::CamParm.angle_of_pitch_ptz);
        query_parm.exec(insert);
    }
    else
    {
        QString select_all_sql = "select * from CAM_CONFIG";
                query_parm.prepare(select_all_sql);
        if(!query_parm.exec())
        {
            myWarning() << query_parm.lastError();
        }
        else
        {
            while(query_parm.next()){
                AppConfig::CamParm.cam_enable = query_parm.value(1).toBool();
                AppConfig::CamParm.cam_ip = query_parm.value(2).toString();
                AppConfig::CamParm.cam_pass = query_parm.value(3).toString();
                AppConfig::CamParm.cam_port = query_parm.value(4).toInt();
                AppConfig::CamParm.cam_user = query_parm.value(5).toString();
                AppConfig::CamParm.cur_realtime_channel = query_parm.value(6).toInt();
                AppConfig::CamParm.is_ptz_direction_same_with_sonar = query_parm.value(7).toBool();
                AppConfig::CamParm.is_save_snapshot = query_parm.value(8).toBool();
                AppConfig::CamParm.angle_of_pitch_ptz = query_parm.value(9).toDouble();
            }
        }
    }
    cmd_str = QString("SELECT name FROM sqlite_master WHERE type='table' AND name='DEV_INFO'");
    //查询是否建立了表格，没有建立则建立
    if(!(query_parm.exec(cmd_str) && query_parm.next()))  //如果表格没有被创建
    {
        myWarning() << "DEV_INFO table does not exist";
        //定义一条创建表的sql语句 表名: MYPARMCONFIG 表中的字段: id  name age
        QString sqlCreate = QString("create table DEV_INFO(id INTEGER PRIMARY KEY,"
                                    "dev_name int,dev_ip QString,dev_netmask QString,dev_gateway QString);");
        query_parm.exec(sqlCreate);
        QString insert = QString("insert into DEV_INFO(dev_name,dev_ip,dev_netmask,dev_gateway) "
                                 "values('%1','%2','%3','%4')")
                .arg(AppConfig::DevInfo.dev_name).arg(AppConfig::DevInfo.dev_ip)
                .arg(AppConfig::DevInfo.dev_netmask).arg(AppConfig::DevInfo.dev_gateway);
        query_parm.exec(insert);
    }
    else
    {
        QString select_all_sql = "select * from DEV_INFO";
                query_parm.prepare(select_all_sql);
        if(!query_parm.exec())
        {
            myWarning() << query_parm.lastError();
        }
        else
        {
            while(query_parm.next()){
                AppConfig::DevInfo.dev_name = query_parm.value(1).toString();
                AppConfig::DevInfo.dev_ip = query_parm.value(2).toString();
                AppConfig::DevInfo.dev_netmask = query_parm.value(3).toString();
                AppConfig::DevInfo.dev_gateway = query_parm.value(4).toString();
            }
        }
    }
    cmd_str = QString("SELECT name FROM sqlite_master WHERE type='table' AND name='DEV_HEART'");
    //查询是否建立了表格，没有建立则建立
    if(!(query_parm.exec(cmd_str) && query_parm.next()))  //如果表格没有被创建
    {
        myWarning() << "DEV_HEART table does not exist";
        //定义一条创建表的sql语句 表名: MYPARMCONFIG 表中的字段: id  name age
        QString sqlCreate = QString("create table DEV_HEART(id INTEGER PRIMARY KEY,"
                                    "is_notify_heartbeat int,notify_interval int,notify_url QString);");
        query_parm.exec(sqlCreate);
        QString insert = QString("insert into DEV_HEART(is_notify_heartbeat,notify_interval,notify_url) "
                                 "values('%1','%2','%3')")
                .arg(AppConfig::DevHeart.is_notify_heartbeat).arg(AppConfig::DevHeart.notify_interval)
                .arg(AppConfig::DevHeart.notify_url);
        query_parm.exec(insert);
    }
    else
    {
        QString select_all_sql = "select * from DEV_HEART";
                query_parm.prepare(select_all_sql);
        if(!query_parm.exec())
        {
            myWarning() << query_parm.lastError();
        }
        else
        {
            while(query_parm.next()){
                AppConfig::DevHeart.is_notify_heartbeat = query_parm.value(1).toInt();
                AppConfig::DevHeart.notify_interval = query_parm.value(2).toInt();
                AppConfig::DevHeart.notify_url = query_parm.value(3).toString();
            }
        }
    }
    cmd_str = QString("SELECT name FROM sqlite_master WHERE type='table' AND name='DEV_TCPSEND'");
    //查询是否建立了表格，没有建立则建立
    if(!(query_parm.exec(cmd_str) && query_parm.next()))  //如果表格没有被创建
    {
        myWarning() << "DEV_TCPSEND table does not exist";
        //定义一条创建表的sql语句 表名: MYPARMCONFIG 表中的字段: id  name age
        QString sqlCreate = QString("create table DEV_TCPSEND(id INTEGER PRIMARY KEY,"
                                    "is_send_tcpserver int,tcpserver_addr QString,tcpserver_port int,type int);");
        query_parm.exec(sqlCreate);
        QString insert = QString("insert into DEV_TCPSEND(is_send_tcpserver,tcpserver_addr,tcpserver_port,type) "
                                 "values('%1','%2','%3','%4')")
                .arg(AppConfig::DevTcpSend.is_send_tcpserver).arg(AppConfig::DevTcpSend.tcpserver_addr)
                .arg(AppConfig::DevTcpSend.tcpserver_port).arg(AppConfig::DevTcpSend.type);
        query_parm.exec(insert);
    }
    else
    {
        QString select_all_sql = "select * from DEV_TCPSEND";
                query_parm.prepare(select_all_sql);
        if(!query_parm.exec())
        {
            myWarning() << query_parm.lastError();
        }
        else
        {
            while(query_parm.next()){
                AppConfig::DevTcpSend.is_send_tcpserver = query_parm.value(1).toInt();
                AppConfig::DevTcpSend.tcpserver_addr = query_parm.value(2).toString();
                AppConfig::DevTcpSend.tcpserver_port = query_parm.value(3).toInt();
                AppConfig::DevTcpSend.type = query_parm.value(4).toInt();
            }
        }
    }
    /*============================================  日志信息数据库 =====================================================*/
    logdb = QSqlDatabase::addDatabase("QSQLITE","myLogs.db");
    logdb.setDatabaseName("/home/baolong/Sqls/myLogs.db");
    if(!logdb.open())
    {
        myWarning() << "myLogs.db open error!" << logdb.lastError();
    }
    //访问数据库的操作主要包括：创建表 向数据库中插入数据、删除数据、更新数据、查询数据
    //对于数据库中的表,通常只需要创建一次,而其他的操作可以重复
    QSqlQuery query_log(logdb);//在创建该对象时，系统自动完成跟数据库的关联

    QString cmd_str_log = QString("SELECT name FROM sqlite_master WHERE type='table' AND name='ALARM_LOGS'");
    //查询是否建立了表格，没有建立则建立
    if(!(query_log.exec(cmd_str_log) && query_log.next()))  //如果表格没有被创建
    {
        myWarning() << "ALARM_LOGS table does not exist";
        //定义一条创建表的sql语句 表名: MYPARMCONFIG 表中的字段: id  name age
        QString sqlCreate_log = QString("create table ALARM_LOGS(id INTEGER PRIMARY KEY,"
                                    "nTimeStep int, time QString,db_value float,tde_ratio float,fde_ratio float,"
                                    "horizontal int,vertical int,saving_path QString,files_status int);");
        query_log.exec(sqlCreate_log);
//        QString insert_log = QString("insert into ALARM_LOGS(nTimeStep,time,db_value,tde_ratio,fde_ratio,horizontal,vertical,saving_path,files_status) "
//                                 "values('%1','%2','%3','%4','%5','%6','%7','%8','%9')")
//                .arg(0).arg("2024-06-06_12-00-00").arg(65.07).arg(0.6642).arg(22.4007).arg(357).arg(90).arg("20220511_141444/").arg(1);
//        query_log.exec(insert_log);
    }

    cmd_str_log = QString("SELECT name FROM sqlite_master WHERE type='table' AND name='ALARM_LOGS_INFO'");
    if(!(query_log.exec(cmd_str_log) && query_log.next()))  //如果表格没有被创建
    {
        myWarning() << "ALARM_LOGS_INFO table does not exist";
        //定义一条创建表的sql语句 表名: MYPARMCONFIG 表中的字段: id  name age
        QString sqlCreate_log = QString("create table ALARM_LOGS_INFO(id INTEGER PRIMARY KEY,"
                                    "cur_logs_sumnum int, cur_logs_number int);");
        query_log.exec(sqlCreate_log);
        QString insert_log = QString("insert into ALARM_LOGS_INFO(cur_logs_sumnum,cur_logs_number) "
                                 "values('%1','%2')")
                .arg(0).arg(0);
        query_log.exec(insert_log);
    }
    else
    {
        QString select_all_sql = "select * from ALARM_LOGS_INFO";
        query_log.prepare(select_all_sql);
        if(!query_log.exec())
        {
            myWarning() << query_log.lastError();
        }
        else
        {
            while(query_log.next()){
                AppConfig::AlarmInfo.cur_logs_sumnum = query_log.value(1).toInt();
                AppConfig::AlarmInfo.cur_logs_number = query_log.value(2).toInt();
            }
        }
    }
}

bool save_signalB(void)//保存信号零点B值
{
    QSqlQuery query(parmdb);
    query.prepare("update ALARM_CONFIG set signal_b = :signal_b  where id = :id");
    query.bindValue(":signal_b", AppConfig::AlarmParm.signal_b);
    query.bindValue(":id", 1);
    query.exec();
    return true;
}

bool insertLog_Sql(void)//插入日志
{
    QSqlQuery query(logdb);

    AppConfig::AlarmInfo.cur_logs_number++;
    AppConfig::AlarmInfo.cur_logs_sumnum++;

    if(AppConfig::AlarmInfo.cur_logs_number > MAX_LOGS_NUM)
    {
        if(deleteLog_Sql(AppConfig::AlarmInfo.cur_logs_number - MAX_LOGS_NUM))
        {
            AppConfig::AlarmInfo.cur_logs_sumnum--;  //如果删除上一条日志
        }
    }
    if(AppConfig::AlarmInfo.cur_logs_number > MAX_SAVEFILE_NUM)
    {
        //根据编号找到之前的文件夹
        QString path_l;
        QString select_all_sql = QString("select * from ALARM_LOGS where id = '%1'").arg(AppConfig::AlarmInfo.cur_logs_number - MAX_SAVEFILE_NUM);
        query.prepare(select_all_sql);
        if(!query.exec())
        {
            myWarning() << query.lastError();
        }
        else
        {
            while(query.next()){
                path_l = query.value(8).toString();
            }
        }
        QString path = QString("/home/baolong/Logs/%1").arg(path_l);
        if(clearDir(path))
        {
            QString update_sql = QString("update ALARM_LOGS set files_status = :files_status where id = '%1'").arg(AppConfig::AlarmInfo.cur_logs_number - MAX_SAVEFILE_NUM);
            query.prepare(update_sql);
            query.bindValue(":files_status", 0);
            query.exec();
        }
    }
    QString cmd = QString("insert into ALARM_LOGS values('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', '%10')").arg(AppConfig::AlarmInfo.cur_logs_number).arg(AppConfig::AlarmInfo.nTimeStep).arg(AppConfig::AlarmInfo.time)
            .arg(AppConfig::AlarmInfo.db_value).arg(AppConfig::AlarmInfo.tde_ratio).arg(AppConfig::AlarmInfo.fde_ratio).arg(AppConfig::AlarmInfo.horizontal)
            .arg(AppConfig::AlarmInfo.vertical).arg(AppConfig::AlarmInfo.saving_path).arg(AppConfig::AlarmInfo.files_status);
    if(query.exec(cmd))
    {
        //保存表ALARM_LOGS_INFO
        query.prepare("update ALARM_LOGS_INFO set cur_logs_number = :cur_logs_number,cur_logs_sumnum = :cur_logs_sumnum where id = :id");
        query.bindValue(":cur_logs_number", AppConfig::AlarmInfo.cur_logs_number);
        query.bindValue(":cur_logs_sumnum", AppConfig::AlarmInfo.cur_logs_sumnum);
        query.bindValue(":id", 1);
        query.exec();
        //myWarning() << "cur_logs_number = " << AppConfig::AlarmInfo.cur_logs_number;
        return true;
    }
    else
    {
        myWarning() << "insertLog_Sql error!";
        return false;
    }
}

bool deleteLog_Sql(int id)
{
    QSqlQuery query(logdb);    //执行查询语句
    QString cmd = QString("delete from ALARM_LOGS where id='%1'").arg(id);
    if(query.exec(cmd)) {
        return true;
    }
    myWarning() << "deleteLog_Sql error!" << id;
    return false;
}

bool clearDir(QString path)
{
    if (path.isEmpty())
    {
        return false;
    }
    QDir dir(path);
    if (!dir.exists())
    {
        return false;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
    QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
    //遍历文件信息
    foreach(QFileInfo file, fileList)
    {
        // 是文件，删除
        if (file.isFile())
        {
            file.dir().remove(file.fileName());
        }
        else // 递归删除
        {
            clearDir(file.absoluteFilePath());
            file.dir().rmdir(file.absoluteFilePath());
        }
    }
    dir.rmdir(path);
    //myWarning() << "clearDir path = " << path;
    return true;
}

void Float2str(char* at, float tmp)
{
    union CHAR_FLOAT_UN charFloat;

    charFloat.floatData = tmp;
    at[0] = charFloat.chData[3];//高字节在前
    at[1] = charFloat.chData[2];
    at[2] = charFloat.chData[1];
    at[3] = charFloat.chData[0];
}






















