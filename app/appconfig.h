#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "QString"

#include <QSqlDatabase>     //可以用来连接和打开数据库
#include <QSqlQuery>        //可以用来真正访问操作数据库
#include <QSqlError>
#include <QDateTime>
#include <QLocale>

#include <QDir>

#define MAX_LOGS_NUM                1000  //最大的日志个数
#define MAX_SAVEFILE_NUM            200  //最大保存的文件个数

#define myWarning()     qWarning()
#define myInfo()        qInfo()

//报警参数配置信息
typedef struct
{
    int id;                     //日志编号
    int nTimeStep;              //时间戳
    QString time;               //报警事件，格式为”yyyy-MM-dd_HH-mm-SS”
    float db_value;               //报警时声音分贝值
    float tde_ratio;            //报警时的时域能量比
    float fde_ratio;            //报警时，分析频段的能量的占比
    int horizontal;             //水平方位角
    int vertical;               //垂直方位角
    QString saving_path;        //报警文件保存路径，具体内容参见注释。
    bool files_status;          //报警文件存储状态，1为已保存，0为已删除

    int cur_logs_sumnum;            //日志总数量
    int cur_logs_number;            //当前日志编号
} ALARM_LOGINFO;

//报警参数配置信息
typedef struct
{
    QString notify_url;             //报警消息回传地址
    bool is_send_alarm;             //是否转发报警信息
    bool is_send_realtime_value;    //是否发送实时分析数据
    float db_limit_day;                 //白天声音分贝报警门限
    float db_limit_night;                 //白天声音分贝报警门限
    float tde_ratio_limit;          //时域能量比门限
    float fde_ratio_limit;          //频域能量比报警门限
    int freq_up_limit;              //分析频带上限[截止频率]，不得高于10KHz，至少大于下限100HZ
    int freq_down_limit;            //分析频带下限[起始频率]，至少小于上限100HZ

    int signal_b;                       //信号的零点校准值
} ALARM_PARMCONFIG;

//球机参数
typedef struct
{
    bool cam_enable;                //球机使能
    QString cam_ip;                 //球机ip
    QString cam_pass;               //球机控制密码
    int cam_port;                   //球机端口
    QString cam_user;               //球机控制账户名
    int cur_realtime_channel;       //实时信道
    bool is_ptz_direction_same_with_sonar;//球机转动方向是否与声呐设备标注方向一致
    bool is_save_snapshot;          //是否进行报警抓拍

    float angle_of_pitch_ptz;           //俯仰角设置30°40°... 90°  对应pzt控制0.0-1.0

} CAM_PARMCONFIG;

//设备信息参数
typedef struct
{
    QString dev_name;               //设备名称（可修改）
    QString dev_ip;                 //设备IP（可修改）
    QString dev_netmask;            //设备子网掩码（可修改）
    QString dev_gateway;            //设备网关（可修改）

    QString dev_type;               //设备种类（不可修改）
    QString dev_version;            //服务版本（不可修改）
    QString upgrade_time;           //当前版本发布时间（不可修改）
    QString web_version;            //当前web 服务版本（不可修改）
    QString equip_number;           //机器标识码（不可修改）
} DEV_INFOCONFIG;

//手动控制摄像头参数
typedef struct
{
    bool is_ctrl_cam;        //是否进行手动控制摄像头，发送true:手动,false:自动
    float rotation_angle;      //旋转角度0-360°设置0°10°...  360°
    float angle_of_pitch;      //俯仰角设置30°40°... 90°

} SELF_CTRL;

//设备心跳参数
typedef struct
{
    bool is_notify_heartbeat;        //回传开关（0-不回传；1-回传信息）
    int notify_interval;            //回传间隔
    QString notify_url;             //回传地址
} DEV_HEART;

typedef struct
{
    bool is_send_tcpserver;         //是否进行tcp发送数据（0-不回传；1-回传信息）
    QString tcpserver_addr;         //服务器地址
    int tcpserver_port;             //服务器端口
    int type;                       //类型
} DEV_TCPSEND;

class AppConfig
{
public:
    //搜索配置
    static QString SearchDeviceADDR;  //指定设备地址

    static int SearchInterval;      //搜索间隔(单位毫秒)

    static int SearchTimeout;       //超时时间(单位毫秒)

    static bool SearchClear;        //自动清空(每次都重新搜索)

    static bool NetworkError;       //网络异常标志

    static bool is_test_mic;       //通道麦克风测试

    static uint8_t test_result[16];       //通道麦克风测试结果

    static bool TcpClientNetError;       //网络异常标志

    static bool ProfilesError;       //网络异常标志

    static ALARM_PARMCONFIG AlarmParm;//报警参数配置信息

    static CAM_PARMCONFIG CamParm;  //球机参数

    static DEV_INFOCONFIG DevInfo;  //设备信息

    static ALARM_LOGINFO AlarmInfo;  //当前报警信息

    static DEV_HEART DevHeart;      //心跳参数

    static DEV_TCPSEND DevTcpSend;      //tcp发送参数

    static bool TcpconnectError;    //TCP连接

    static SELF_CTRL SelfCtrl;      //控制摄像头

    static bool is_calibration;     //是否进行方向校准

    static bool is_update_code;     //是否进行系统升级
};

union CHAR_FLOAT_UN{
    char chData[4];
    float floatData;
};

extern QSqlDatabase parmdb;
extern QSqlDatabase logdb;

void mySqlDatebase_Init(void);      //数据库初始化

bool save_signalB(void);            //保存信号零点B值

bool insertLog_Sql(void);           //数据库插入日志

bool deleteLog_Sql(int id);         //从数据库删除日志

bool clearDir(QString path);        //删除文件夹和文件

unsigned short CRC16(unsigned char *puchMsg, unsigned short usDataLen);

void Float2str(char* at, float tmp);//float转str

#endif // APPCONFIG_H
