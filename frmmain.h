#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QWidget>
/* ===***===***===***===***http网络通信***===***===***===***=== */
#include "httpserver.h"
#include "tcpserver.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QTimer>
#include <QTime>
#include <QMetaObject>
#include <QProcess>

#include <QDateTime>
#include <QFileDialog>

#include "onvifhead.h"
#include "onvifsearch.h"
#include "onvifdevice.h"
#include "appconfig.h"
#include "tcpserver.h"
#include "tcpclient.h"

#include "led.h"
#include <QFile>
#include <QTextStream>
#include <QtCore>
#include <QQueue>
#include <QtGlobal>

/* ===***===***===***===***采样频率***===***===***===***=== */
#define SAMPLERATE       25000       //采样频率
#define NUMSAMPLES       u16_DATA_LENGTH*AUDIO_NUM_1S*10      //采样个数

typedef struct
{
    int16_t M_data[16][ONE_LENGTH];        //2500
} AUDIO_INFO;

// 定义你的结构体
typedef struct
{
    int16_t audio[ONE_LENGTH];
}RECORD;
typedef struct
{
    RECORD record[16];
}RECORD_QUEUE;

typedef enum
{
    STEP_ERROR = 0,
    STEP_START,
    STEP_PTZ_CTL,
    STEP_IMAGE_1S,
    STEP_SAVE_AUDIO,
    STEP_SAVE_LOG,
    STEP_SELF_CTRL,
    STEP_JIAOZHUN,
    STEP_END,
}STEP_ENUM;

//实时运行数据信息
typedef struct
{
    bool start_flag = false;    //任务开始标志
    float db_value = 0;         //声音分贝值
    float tde_ratio = 0;        //时域能量比
    float fde_ratio = 0;        //分析频段的能量的占比
    uint8_t channel;            //当前触发的麦克风

    bool task_flag = false;     //是否在任务中，如果在任务中，跳过本次计算
    int task_step = 0;
    int step_count = 0;

    int max_db_chn = 0;
    float max_db_value = 0;      //1s内最大分贝值
    float max_fdr_value = 0;
    float max_tdr_value = 0;
} REAL_DATA;

namespace Ui {
class frmMain;
}

class frmMain : public QWidget
{
    Q_OBJECT

public:

    friend class frmIpcPtz;

    frmMain(QWidget *parent = nullptr);

    ~frmMain();

    static void delay_ms(unsigned int msec);//延时功能

private:

    QQueue<RECORD_QUEUE> recordQueue; // 设置最大容量为100

    QNetworkAccessManager manager;

    OnvifSearch *onvifSearch;       /* onvif搜索类对象 */

    QList<OnvifDevice *> devices;   /* 搜索回来的设备对象集合 */

    QList<QString> ipv4s;           /* 搜索回来的设备IP地址集合 */

    QTimer *Main_ControlTimer;      /* 事件定时器100ms */

    QTimer *MinorTimer;             /* 事件定时器1s */

    /* ===***===***===***===***网络通信***===***===***===***=== */
    TcpClient client;

    HttpServer *httpServer;

    TcpServer tcpServer;

    REAL_DATA realData;

    QMutex file_mutex;

    AUDIO_INFO cur_audioInfo;       //一帧16通道完整数据-0.1s

private:
    void alarm_backRequest();

    void file_backRequest();

    void real_backRequest();

    void getNetworkInterfaceIP();

    bool get_mem_usage__();

    void init_delay_filter(void);

    void hubei_TxData(void);

    bool removeDirRecursively(const QString &dirPath);

    void cleanOldFiles(const QString &directoryPath, int maxDays);
private slots:
    void replyFinished(QNetworkReply *reply);   /* http服务端回应的消息 */

    OnvifDevice *getCurrentDevice();/* 获取当前设备 */

    void initForm();                /* 初始化 */

    QString getProfileToken();      /* 获取当前配置文件 */

    void clear();                   /* 清空数据 */

    void receiveError(const QString &data);    /* 报错日志 */

    void Main_ControlEvent();       /* 事件处理定时器，led */

    void Minor_ControlEvent();       /* 事件处理定时器，led */

    /* ===***===***===***===***网络通信***===***===***===***=== */
    void write_audio_wav(QString filename, int sampleRate, int numSamples, uint8_t channel);

    void tcpReceivedData(const QByteArray &sensor_data); /* TCP通讯接收数据 */

    void tcp_clientConnect_slot();  /* tcp客户端连接 */
private slots:
    void searchFinsh();             /* 搜索设备完成 */

    void searchDevice(bool one, int interval = 0);              /* 搜索设备 */

    void GetMedia_clicked();        /* 获取设备信息（device） */

    void GetSnapImage_clicked(QString filename);    /* 获取图片 */

    void setHomePosition_slot();         /* 球机方向校准 */

    void ptzControl_slot();                 /* 手动控制球机 */

    void time_sync_slot();           /* 获取球机时间并同步到设备上 */

    void ptzControl(quint8 type, qreal x, qreal y, qreal z);    /* 控制摄像头 */

    void ptzControl(int position, float pitch_angle);  /* 控制摄像头 */

    void receiveDevice(const OnvifDeviceInfo &deviceInfo);      /* 搜索到设备返回的信息 */
};
#endif // FRMMAIN_H
