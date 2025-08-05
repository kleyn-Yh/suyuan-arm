#include "frmmain.h"

frmMain::frmMain(QWidget *parent)
    : QWidget(parent)
{
    /* 初始化sql */
    mySqlDatebase_Init();

    /* 获取eth1的addr */
    getNetworkInterfaceIP();
    /* 对象实例化 */
    Main_ControlTimer = new QTimer(this);
    connect(Main_ControlTimer, SIGNAL(timeout()), this, SLOT(Main_ControlEvent()));
    Main_ControlTimer->start(10);         //开启100ms定时器

    MinorTimer = new QTimer(this);
    connect(MinorTimer, SIGNAL(timeout()), this, SLOT(Minor_ControlEvent()));
    MinorTimer->start(1000);         //开启1s定时器

    /* http服务器实例化 */
    httpServer = new HttpServer;
    //connect(httpServer, SIGNAL(pram_syncToMCU_signal()),this, SLOT(pram_syncToMCU_slot()));
    connect(httpServer, SIGNAL(SetHomePosition_signal()), this, SLOT(setHomePosition_slot()));
    connect(httpServer, SIGNAL(time_sync_signal()), this, SLOT(time_sync_slot()));
    connect(httpServer, SIGNAL(ptzControl_signal()),this, SLOT(ptzControl_slot()));
    connect(httpServer, SIGNAL(tcpclientconnect_signal()),this, SLOT(tcp_clientConnect_slot()));

    /* tcp客户端初始化*/
    if((AppConfig::DevTcpSend.is_send_tcpserver == true)&&(AppConfig::DevTcpSend.type == 0x01))
    {
        client.connectToServer();
    }

    /* tcp服务器初始化*/
    tcpServer.startServer(TCP_PORT); // 指定监听端口号
    //connect(&tcpServer, SIGNAL(receivedData(QByteArray)), this, SLOT(tcpReceivedData(QByteArray)));
    connect(&tcpServer.msgPackage, SIGNAL(sigPkgReady(QByteArray)), this, SLOT(tcpReceivedData(QByteArray)));

    this->initForm();               //初始化ONVIF
    searchDevice(true, 1000);             //单播搜索设备
    //关联请求完毕信号finished(QNetworkReply* )
    connect(&manager, &QNetworkAccessManager::finished,this, &frmMain::replyFinished);

    myInfo() << "设备初始化完成，开始工作！";
}

frmMain::~frmMain()
{
    delete Main_ControlTimer;
    delete MinorTimer;
    delete httpServer;
}

void frmMain::alarm_backRequest()
{
    QUrl url = QUrl(AppConfig::AlarmParm.notify_url);
    QNetworkRequest request(url);//创建一个请求对象

    QJsonObject obj;
    obj.insert("message_type", "alarm_msg");
    obj.insert("alarm_time", AppConfig::AlarmInfo.time);
    obj.insert("db_value", AppConfig::AlarmInfo.db_value);
    obj.insert("dev_name", AppConfig::DevInfo.dev_name);
    obj.insert("fde_ratio", AppConfig::AlarmInfo.fde_ratio);
    obj.insert("tde_ratio", AppConfig::AlarmInfo.tde_ratio);
    obj.insert("files_status", 0);
    obj.insert("horizontal", AppConfig::AlarmInfo.horizontal);
    obj.insert("vertical", AppConfig::AlarmInfo.vertical);
    obj.insert("saving_path", AppConfig::AlarmInfo.saving_path);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    //发送请求
    request.setRawHeader("Content-Type", "application/json");
    manager.post(request,json);
}

void frmMain::file_backRequest()
{
    QUrl url = QUrl(AppConfig::AlarmParm.notify_url);
    QNetworkRequest request(url);//创建一个请求对象
    QJsonObject obj;
    obj.insert("message_type", "alarm_files_save_report");
    obj.insert("dev_name", AppConfig::DevInfo.dev_name);
    obj.insert("save_status", AppConfig::AlarmInfo.files_status);
    obj.insert("file_path", AppConfig::AlarmInfo.saving_path);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    //发送请求
    request.setRawHeader("Content-Type", "application/json");
    manager.post(request,json);
}

void frmMain::real_backRequest()
{
    int alarm_timestamp  = QDateTime::currentDateTime().toTime_t(); //返回秒级时间戳
    QDateTime sysTime = QDateTime::fromTime_t(alarm_timestamp);
    QString alarm_time = sysTime .toString("yyyyMMdd_hhmmss");//格式化时间
    QUrl url = QUrl(AppConfig::AlarmParm.notify_url);
    QNetworkRequest request(url);//创建一个请求对象
    QJsonObject obj;
    obj.insert("message_type", "realtime_value");
    obj.insert("max_db_chn", realData.max_db_chn);
    obj.insert("dev_name", AppConfig::DevInfo.dev_name);
    obj.insert("alarm_timestamp", alarm_timestamp);
    obj.insert("alarm_time", alarm_time);
    obj.insert("max_db_value", realData.max_db_value);
    obj.insert("max_fdr_value", realData.max_fdr_value);
    obj.insert("max_tdr_value", realData.max_tdr_value);
    QJsonDocument doc(obj);
    QByteArray json = doc.toJson();//json数据转文本格式字符串
    //发送请求
    request.setRawHeader("Content-Type", "application/json");
    manager.post(request,json);
}

void frmMain::getNetworkInterfaceIP()
{
    QList<QNetworkAddressEntry> list;
    QHostAddress addr;
    QNetworkInterface interface = QNetworkInterface::interfaceFromName("eth1");
    list = interface.addressEntries();
    if(!list.isEmpty())
    {
        addr = list.at(0).ip();
        AppConfig::DevInfo.dev_ip = addr.toString();
        return;
    }
     myWarning() << "No IP address found";
}

void frmMain::replyFinished(QNetworkReply *reply)
{
    if (reply->error())                         // <1>判断有没有错误
    {
        myWarning() << reply->errorString();
        reply->deleteLater();
        return;
    }
    int statusCode  = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //myWarning() << "statusCode:" << statusCode;    // <2>检测网页返回状态码，常见是200，404等，200为成功
    if (statusCode >= 200 && statusCode <300)   // <3>判断是否需要重定向
    {
        // 准备读数据
        QTextCodec *codec = QTextCodec::codecForName("utf8");
        QString all = codec->toUnicode(reply->readAll());
        //myWarning() << "接收到的数据" << all;
        reply->deleteLater();                   // <4>数据读取完成之后，清除reply
        reply = nullptr;
    }
    else
    {
        myWarning() << "reply error";
    }
}
/* ***********************
 * @功	能	初始化onvif
 * @参	数	无
 * ***********************/
void frmMain::initForm()
{
    //实例化onvif搜索,绑定信号槽
    onvifSearch = new OnvifSearch(this);

    connect(onvifSearch, SIGNAL(receiveError(QString)), this, SLOT(receiveError(QString)));
    connect(onvifSearch, SIGNAL(receiveDevice(OnvifDeviceInfo)), this, SLOT(receiveDevice(OnvifDeviceInfo)));
}
/* ***********************
 * @功	能	搜索设备
 * @参	数	one ， interval
 * ***********************/
void frmMain::searchDevice(bool one, int interval)
{
    if (interval <= 0) {
        //计算延时时间
        interval = AppConfig::SearchInterval * 4 + 1000;
        //限定最小结束时间 有时候设备很多需要一定时间排队处理数据
        interval = interval < 3000 ? 3000 : interval;
        //单播搜索必须自动清空
        if (one) {
            AppConfig::SearchClear = true;
            interval = 1000;
        }
    }
    //清空数据
    clear();

    //设置搜索过滤条件和间隔
    QString localIP = AppConfig::DevInfo.dev_ip;
    QString deviceIP = AppConfig::CamParm.cam_ip;
    onvifSearch->setSearchFilter("none");
    onvifSearch->setSearchInterval(AppConfig::SearchInterval);
//    onvifSearch->search(localIP, deviceIP, one);
    onvifSearch->search(localIP, one ? deviceIP : "");
    //禁用后延时恢复
//    ui->tabSearch->setEnabled(false);
    myInfo() << QString("正在搜索, 请稍等 %1 秒...").arg(interval / 1000);
    QTimer::singleShot(interval, this, SLOT(searchFinsh()));
}
/* ***********************
 * @功	能	清理
 * @参	数	无
 * ***********************/
void frmMain::clear()
{
    if (AppConfig::SearchClear) {
        qDeleteAll(devices);
        devices.clear();
        ipv4s.clear();
    }
}
/* ***********************
 * @功	能	搜索设备完成
 * @参	数	无
 * ***********************/
void frmMain::searchFinsh()
{
    AppConfig::SearchDeviceADDR = "";
    //排序后添加到下拉框
//#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
//    std::sort(ipv4s.begin(), ipv4s.end());
//#else
    std::sort(ipv4s.begin(),ipv4s.end());
//#endif
    foreach (QString ipv4, ipv4s) {
        QStringList list = ipv4.split("|");
        AppConfig::SearchDeviceADDR.append(list.at(2));
    }
    myInfo() << QString("共搜索到 %1 个设备").arg(devices.size());
    myInfo() << AppConfig::SearchDeviceADDR;
    //立即一键获取
    GetMedia_clicked();
}
/* ***********************
 * @功	能	接收设备信息，添加设备
 * @参	数	deviceInfo
 * ***********************/
void frmMain::receiveDevice(const OnvifDeviceInfo &deviceInfo)
{
    //如果已经存在则不添加
    foreach (OnvifDevice *device, devices) {
        if (device->getOnvifAddr() == deviceInfo.onvifAddr) {
            myWarning() << "设备已存在" << endl;
            return;
        }
    }

    QString addr = deviceInfo.onvifAddr;
    QString ip = deviceInfo.deviceIp;

    //实例化onvif设备类并绑定信号槽
    OnvifDevice *device = new OnvifDevice(this);
    connect(device, SIGNAL(receiveError(QString)), this, SLOT(receiveError(QString)));

    //将onvif地址发送过去
    device->setOnvifAddr(addr);
    //将当前广播搜索返回的设备信息一起打包发给设备类
    device->setDeviceInfo(deviceInfo);

    devices << device;
    ipv4s << QString("%1|%2|%3").arg(OnvifHelper::ipv4StringToInt(ip)).arg(ip).arg(addr);
    //myWarning() << "ipv4s" << ipv4s << endl;
}
/* ***********************
 * @功	能	报错日志
 * @参	数	data
 * ***********************/
void frmMain::receiveError(const QString &data)
{
    myWarning() << "frmMain::receiveError";
    myWarning() << data << endl;
    if(data.contains("Network access is disabled")){
        AppConfig::NetworkError = true;
    }
}
/* ***********************
 * @功	能	获取当前设备
 * @参	数	无
 * ***********************/
OnvifDevice *frmMain::getCurrentDevice()
{
    QString addr = AppConfig::SearchDeviceADDR;
    if (addr.isEmpty()) {
        myWarning() <<"addr.isEmpty()" << endl;
        AppConfig::NetworkError = true;
        return 0;
    }
    //移到这里来设置用户信息,以便每次可以动态对每个设备进行重新设置用户
    //QString userName = AppConfig::CamParm.cam_user;
    //QString userPwd = AppConfig::CamParm.cam_pass;
    QString userName = "baolong";
    QString userPwd = "AHBL2023";
    foreach (OnvifDevice *device, devices) {
        if (device->getOnvifAddr() == addr) {
            device->setUserInfo(userName, userPwd);
            return device;
        }
        myWarning() <<"Error:" << device->getOnvifAddr();
    }
    AppConfig::NetworkError = true;
    myWarning() <<"Error getCurrentDevice" << endl;
    return 0;
}
/* ***********************
 * @功	能	获取球机时间同步到系统
 * @参	数	无
 * ***********************/
void frmMain::time_sync_slot()
{
    if(AppConfig::NetworkError == true)
    {
        return;
        myWarning() << "球机离线！！！" << endl;
    }
    QStringList arg;
    QString date_time;
    OnvifDevice *device = getCurrentDevice();
    if (device) {
        date_time = device->getDateTime();
        date_time = date_time.left(19);
    }
    //soft
    arg << "-s" << date_time;
    QProcess::execute("date", arg);

    arg.clear();
    arg << "-w";
    QProcess::execute("hwclock", arg);

    arg.clear();
    arg << "-w" << "-f" << "/dev/rtc0";
    QProcess::execute("hwclock", arg);
}
/* ***********************
 * @功	能	摄像头方向校准
 * @参	数	无
 * ***********************/
void frmMain::setHomePosition_slot()
{
    OnvifDevice *device = getCurrentDevice();
    if(device) {
        device->ptzPreset(4, getProfileToken());
    }
}
/* ***********************
 * @功	能	手动控制球机
 * @参	数	无
 * ***********************/
void frmMain::ptzControl_slot()
{
    ptzControl(1, AppConfig::SelfCtrl.rotation_angle, AppConfig::CamParm.angle_of_pitch_ptz, 0.0);
}

/* ***********************
 * @功	能	获取文件路径
 * @参	数	无
 * ***********************/
QString frmMain::getProfileToken()
{
    QString file = "Profile_1";
    return file;
}
/* ***********************
 * @功	能	获取设备信息
 * @参	数	无
 * ***********************/
void frmMain::GetMedia_clicked()
{
    OnvifDevice *device = getCurrentDevice();
    if (device) {
        //获取服务文件 onvif1.0一般用getCapabilities 2.0用getServices
        device->getServices();
        //如果没有获取到地址则说明设备可能是 onvif1.0 重新发送1.0对应的请求数据
        if (device->getMediaUrl().isEmpty() || device->getPtzUrl().isEmpty()) {
            device->getCapabilities();
        }
        //判断device是否正常
        if(device->getMediaUrl().contains("/Media")
            &&(device->getPtzUrl().contains("/PTZ"))
                &&(device->getImageUrl().contains("/Imaging"))
                    &&(device->getEventUrl().contains("/Events"))){
            AppConfig::NetworkError = false;
        }else{
            AppConfig::NetworkError = true;
            myWarning() << "设备异常！" << endl;
        }
        //获取配置文件profile
        myInfo() << "device.getMediaUrl:" << device->getMediaUrl();
        myInfo() << "device.getPtzUrl:" << device->getPtzUrl();
        myInfo() << "device.getImageUrl:" << device->getImageUrl();
        myInfo() << "device.getEventUrl:" << device->getEventUrl();

        QList<OnvifProfileInfo> profiles = device->getProfiles();
        foreach (OnvifProfileInfo profile, profiles) {
            myInfo() << profile.token;
            AppConfig::ProfilesError = false;
        }

    }
}
/* ***********************
 * @功	能	获取图片
 * @参	数	无
 * ***********************/
void frmMain::GetSnapImage_clicked(QString filename)
{
    if((AppConfig::NetworkError == true)||(AppConfig::CamParm.is_save_snapshot == false))    //球机设备异常
    {
        return;
    }
    OnvifDevice *device = getCurrentDevice();
    if(device) {
        // 将文件名设置为当前时间
        //QString fileName = "image-" + dateTime + ".jpg";
        QImage image = device->snapImage(getProfileToken());
        if(!image.isNull()) {
            //保存文件
            QString filePath = QString("/home/baolong/Logs/%1").arg(AppConfig::AlarmInfo.saving_path);
            QDir dir(filePath);
            if(!dir.exists()){
                bool ok = dir.mkpath(filePath);
                if(!ok){
                    return;
                }
            }
            QString fileName = QString("/home/baolong/Logs/%1%2.jpg").arg(AppConfig::AlarmInfo.saving_path).arg(filename);
            image.save(fileName, "jpg");
            myInfo() << QString("图片大小 -> %1 x %2").arg(image.width()).arg(image.height());
            //等比缩放一下
        }
    }
}
/* ***********************
 * @功	能	控制摄像头
 * @参	数	绝对点位
 * ***********************/
void frmMain::ptzControl(quint8 type, qreal x, qreal y, qreal z)
{
    OnvifDevice *device = getCurrentDevice();
    if(device) {
        device->ptzControl(type, getProfileToken(), x, y, z);
        QString strX = QString::number(x, 'f', 1);
        QString strY = QString::number(y, 'f', 1);
        QString strZ = QString::number(z, 'f', 1);
        myInfo() << QString("云台控制 -> x: %1  y: %2  z: %3").arg(strX).arg(strY).arg(strZ);
    }
}
/*
 * @功	能	控制摄像头
 * @参	数	position
 * */
void frmMain::ptzControl(int position, float pitch_angle)
{
    if((AppConfig::NetworkError == true)||(AppConfig::CamParm.is_save_snapshot == false))    //球机设备异常
    {
        return;
    }
    //获取云台控制类型
    quint8 type = 1;//1:绝对运动，2：相对运动，3：连续运动
    qreal x = 0;
    if(AppConfig::CamParm.is_ptz_direction_same_with_sonar == false)
    {
        if(position > 0)
        {
            position = 16 - position;
        }
    }
    if(position < 8){
        x = position * 0.125;
        ptzControl(type, x, pitch_angle, 0.0);
    }
    else if((8 <= position) &&(position < 16)){
        x = (16 - position) * 0.125;
        ptzControl(type, -x, pitch_angle, 0.0);
    }
    else{
        ptzControl(type, 0.0, pitch_angle, 0.0);
    }
}
/*
 * @功	能	延时函数配置
 * @参	数	延时（单位：毫秒）
 * */
void frmMain::delay_ms(unsigned int msec)
{
    QTime reachTime = QTime::currentTime().addMSecs(msec);
    //用while循环不断比对当前时间与我们设定的时间
    while(QTime::currentTime()<reachTime){
        QApplication::processEvents(QEventLoop::AllEvents,100);
    }
}
/* ===***===***===***===***网络通信***===***===***===***=== */
void frmMain::write_audio_wav(QString filename, int sampleRate, int numSamples, uint8_t channel)
{
    uint16_t iiii = 0;
    int16_t sample = 0;
    //保存文件
    file_mutex.lock();
    QString filepath = QString("/home/baolong/Logs/%1").arg(AppConfig::AlarmInfo.saving_path);
    QString fileName = QString("%1%2.wav").arg(filepath).arg(filename);
    QByteArray byteArray = fileName.toUtf8();
    const char* filePath = byteArray.constData();
    FILE* file = fopen(filePath, "wb");
    if (!file) {
        file_mutex.unlock();
        myWarning() << "Failed to open file for writing: " << filePath;
        return;
    }
    short numChannels = 1;
    int bitsPerSample = 16;
    int bytesPerSample = bitsPerSample / 8;
    int dataSize = numSamples * numChannels * bytesPerSample;
    int totalSize = dataSize + 36;
    fwrite("RIFF", 4, 1, file);
    fwrite(&totalSize, sizeof(int), 1, file);
    fwrite("WAVE", 4, 1, file);
    fwrite("fmt ", 4, 1, file);
    int fmtSize = 16;
    fwrite(&fmtSize, sizeof(int), 1, file);
    short audioFormat = 1;
    fwrite(&audioFormat, sizeof(short), 1, file);
    fwrite(&numChannels, sizeof(short), 1, file);
    fwrite(&sampleRate, sizeof(int), 1, file);
    int byteRate = sampleRate * numChannels * bytesPerSample;
    fwrite(&byteRate, sizeof(int), 1, file);
    short blockAlign = numChannels * bytesPerSample;
    fwrite(&blockAlign, sizeof(short), 1, file);
    fwrite(&bitsPerSample, sizeof(short), 1, file);
    fwrite("data", 4, 1, file);
    fwrite(&dataSize, sizeof(int), 1, file);

    for (RECORD_QUEUE value : recordQueue){
        for(int k = 0; k < ONE_LENGTH; k++)
        {
            sample = value.record[channel].audio[k] ;
            fwrite(&sample, sizeof(int16_t), 1, file);
        }
        iiii++;
    }
    fclose(file);
    file_mutex.unlock();
    myInfo() << "success write_audio: " << iiii;
}

void frmMain::tcpReceivedData(const QByteArray &sensor_data)
{
    RECORD_QUEUE record;
    static float dbvalueMax = 0;
    uint16_t signalMax = 0;
    uint16_t channalMax = 0;
    uint16_t signalPlace = 0;
    uint32_t signalSub[16] = {0};
    static uint32_t last_signalSub[16] = {0};
    static uint16_t audio_count = 0;
    static uint16_t file_count = 0;
    uint32_t len = sensor_data.size();
    // 获取当前的时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QTime currentTime = currentDateTime.time();
    // 创建一个6:00的时间
    QTime compareTime1(6, 0);  // 6:00 AM
    QTime compareTime2(22, 0);  // 22:00 AM

    if(len == u8_DATA_LENGTH)
    {
        for(int chnal = 0; chnal < 16; chnal++)
        {
            for(int idx = 0; idx < ONE_LENGTH; idx++)
            {
                cur_audioInfo.M_data[chnal][idx] = (static_cast<int16_t>(sensor_data.data()[(16*idx+chnal)*2]) << 8) | (static_cast<int16_t>(sensor_data.data()[(16*idx+chnal)*2+1]));
            }
            /**************************************消除直流分量**************************************/


            /**************************************LMS自适应滤波**************************************/
            for(int idx = 0; idx < ONE_LENGTH; idx++)
            {
                record.record[chnal].audio[idx] = cur_audioInfo.M_data[chnal][idx];
                signalSub[chnal] += cur_audioInfo.M_data[chnal][idx] * cur_audioInfo.M_data[chnal][idx];
                if(signalMax < cur_audioInfo.M_data[chnal][idx])
                {
                    signalMax = cur_audioInfo.M_data[chnal][idx];
                    signalPlace = chnal;
                }
                if(channalMax < cur_audioInfo.M_data[chnal][idx])
                {
                    channalMax = cur_audioInfo.M_data[chnal][idx];
                }

            }
            if(AppConfig::is_test_mic == true)
            {
                if(channalMax > 1000)
                {
                    AppConfig::test_result[chnal] = 1;
                }
            }
            channalMax = 0;
        }
        if(last_signalSub[signalPlace] != 0){
            realData.max_tdr_value = (float)signalSub[signalPlace]/(float)last_signalSub[signalPlace];
        }
        for(int chnal = 0; chnal < 16; chnal++)
        {
            last_signalSub[chnal] = signalSub[chnal];
        }
        realData.db_value = 20 * log10(signalMax);
        if((realData.start_flag == false)&&(realData.max_tdr_value > AppConfig::AlarmParm.tde_ratio_limit))
        {
            if((currentTime >= compareTime1)&&(currentTime < compareTime2))
            {
                if(realData.db_value > AppConfig::AlarmParm.db_limit_day)
                {
                    if(signalPlace < 16)
                    {
                        if(realData.channel != signalPlace)
                        {
                            realData.channel = signalPlace;
                            AppConfig::AlarmInfo.db_value = realData.db_value;
                            AppConfig::AlarmInfo.horizontal = realData.channel * 22.5 + qrand()%10;
                            AppConfig::AlarmInfo.tde_ratio = realData.max_tdr_value;
                            realData.start_flag = true;
                        }
                    }
                }
            }
            else
            {
                if(realData.db_value > AppConfig::AlarmParm.db_limit_night)
                {
                    if(signalPlace < 16)
                    {
                        if(realData.channel != signalPlace)
                        {
                            realData.channel = signalPlace;
                            AppConfig::AlarmInfo.db_value = realData.db_value;
                            AppConfig::AlarmInfo.horizontal = realData.channel * 22.5 + qrand()%10;
                            AppConfig::AlarmInfo.tde_ratio = realData.max_tdr_value;
                            realData.start_flag = true;
                        }
                    }
                }
            }
        }
        //实时录音缓存到队列中
        if(recordQueue.size() < 100){     //队列小于100
            recordQueue.append(record);
        }else
        {
            recordQueue.dequeue();
            if(recordQueue.size() < 100){
                recordQueue.append(record);
            }else
            {
                recordQueue.dequeue();
                myWarning() << "Queue > 100 ERROR!";
            }
        }
        //计算1s内最大分贝值
        if(dbvalueMax < realData.db_value)
        {
            dbvalueMax = realData.db_value;
            if(signalPlace < 16)
            {
                realData.max_db_chn = signalPlace + 1;
            }
        }
        audio_count++;
        if(audio_count >= AUDIO_NUM_1S)
        {
            realData.max_db_value = dbvalueMax;
            file_count++;
            if(file_count >= 10)
            {
                file_count = 0;
            }
//            myWarning() << "file_count======: " << file_count;
//            myWarning() << "  " << record.record[0].audio[50] << record.record[1].audio[50] << record.record[2].audio[100]<< record.record[3].audio[100] << record.record[4].audio[50] << record.record[5].audio[50]
//                    << record.record[6].audio[50] << record.record[7].audio[50] << record.record[8].audio[50]<< record.record[9].audio[50] << record.record[10].audio[50] << record.record[11].audio[50]
//                    << record.record[12].audio[50] << record.record[13].audio[50] << record.record[14].audio[50]<< record.record[15].audio[50];
            audio_count = 0;
            dbvalueMax = 0;
        }
    }
    else
    {
        qWarning() << "接收数据长度错误，长度为: " << len;
    }
}
/* ***********************
 * @功	能	tcp客户端连接
 * @参	数	无
 * ***********************/
void frmMain::tcp_clientConnect_slot()
{
    if((AppConfig::DevTcpSend.is_send_tcpserver == true)&&(AppConfig::DevTcpSend.type == 0x01))
    {
        client.connectToServer();
    }
    else
    {
        client.disconnectToServer();
    }
}
/* ***********************
 * @功	能	tcp客户端连接
 * @参	数	无
 * ***********************/
unsigned int CRC16_HJ212( unsigned char *puchMsg, unsigned int usDataLen )
{
    unsigned int i,j,crc_reg,check;
    crc_reg = 0xFFFF;
    for(i=0;i<usDataLen;i++)
    {
        crc_reg = (crc_reg>>8) ^ puchMsg[i];
        for(j=0;j<8;j++)
        {
            check = crc_reg & 0x0001;
            crc_reg >>= 1;
            if(check==0x0001)
            {
                crc_reg ^= 0xA001;
            }
        }
    }
    return crc_reg;
}
void frmMain::hubei_TxData(void)
{
    int alarm_timestamp  = QDateTime::currentDateTime().toTime_t(); //返回秒级时间戳
    QDateTime sysTime = QDateTime::fromTime_t(alarm_timestamp);
    QString alarm_time = sysTime .toString("yyyyMMddhhmmss");//格式化时间
    QString randomNumberStr = QString::number(qrand()%1000).rightJustified(3, '0'); // 转换为字符串并补零
    QString QN = alarm_time + randomNumberStr;
    QString middle = QString("QN=%1;ST=23;CN=2093;PW=123456;MN=%2;Flag=5;CP=&&DataTime=%3;Direction=%4&&")
            .arg(QN).arg(AppConfig::DevInfo.dev_name.rightJustified(15, '0')).arg(alarm_time).arg(AppConfig::AlarmInfo.horizontal);
    QByteArray byteArray = middle.toUtf8(); // 或使用 str.toLocal8Bit()
    unsigned char* middle_crc = reinterpret_cast<unsigned char*>(byteArray.data());
    uint16_t dataLenth = middle.length();
    QString lenthStr = QString::number(dataLenth).rightJustified(4, '0'); // 转换为字符串并补零
    QString front = QString("##%1").arg(lenthStr);

    uint16_t crc16 = CRC16_HJ212(middle_crc, dataLenth);
    QString last = QString("%1\r\n").arg(QString::number(crc16, 16).toUpper());

    QString sendData = front + middle + last;

    client.sendMessage(sendData);
}
/*
 * @功	能	主事件处理定时器
 * @参	数	无
 * */
void frmMain::Main_ControlEvent()
{
    if(AppConfig::is_calibration == true)          //进行方向校准
    {
        realData.task_step = STEP_JIAOZHUN;
    }
    else if(AppConfig::SelfCtrl.is_ctrl_cam == true)         //是否手动控制摄像头
    {
        realData.task_step = STEP_SELF_CTRL;
    }
    switch(realData.task_step)
    {
    case STEP_ERROR:
            realData.task_step = STEP_START;
            realData.step_count = 0;
        break;
    case STEP_START:
        if(realData.start_flag == true)     //满足条件，分贝值、时域能量比、频带能量比
        {
            AppConfig::AlarmInfo.nTimeStep  = QDateTime::currentDateTime().toTime_t(); //返回秒级时间戳
            QDateTime sysTime = QDateTime::fromTime_t(AppConfig::AlarmInfo.nTimeStep);
            AppConfig::AlarmInfo.time = sysTime .toString("yyyyMMdd_hhmmss");//格式化时间
            AppConfig::AlarmInfo.saving_path = QString("%1/").arg(AppConfig::AlarmInfo.time);
            AppConfig::AlarmInfo.files_status = true;
            AppConfig::AlarmInfo.vertical = (AppConfig::CamParm.angle_of_pitch_ptz + 0.5) *60 ;
            realData.task_step = STEP_PTZ_CTL;
            if(AppConfig::AlarmParm.is_send_alarm == true)//2、是否回传报警日志
            {
                alarm_backRequest();//到指定的url
            }
            GetSnapImage_clicked("1");        //1、拍第1张图片
        }
        break;
    case STEP_PTZ_CTL:
        if(realData.task_flag != true)      //任务中，不进行信号分析
        {
            realData.task_flag = true;
            ptzControl(realData.channel, AppConfig::CamParm.angle_of_pitch_ptz);    //2、摄像头转动
            realData.task_step = STEP_IMAGE_1S;
            realData.step_count = 0;
        }
        break;
    case STEP_IMAGE_1S:
        if(realData.step_count == 100)        //3、1s后拍摄第2张图片
        {
            GetSnapImage_clicked("2");
        }
        else if(realData.step_count > 200)   //4、2s后拍摄第3张图片
        {
            realData.step_count = 0;
            GetSnapImage_clicked("3");
            realData.task_step = STEP_SAVE_AUDIO;
            break;
        }
        realData.step_count++;
        break;
    case STEP_SAVE_AUDIO:
        if(realData.step_count > 210)
        {
            realData.step_count = 0;
            //保存录音文件
            write_audio_wav("alarm", SAMPLERATE, NUMSAMPLES,realData.channel);
            realData.task_step = STEP_SAVE_LOG;
        }
        realData.step_count++;
        break;
    case STEP_SAVE_LOG:
        insertLog_Sql();                    //1、保存报警日志到数据库
        if(AppConfig::AlarmParm.is_send_alarm == true)//2、是否回传报警日志
        {
            file_backRequest();//到指定的url
        }
        if((AppConfig::DevTcpSend.is_send_tcpserver == true)&&(AppConfig::DevTcpSend.type = 0x01))
        {
            if(AppConfig::TcpClientNetError == false)
            {
                hubei_TxData();
            }
        }
        realData.task_step = STEP_END;
        break;
    case STEP_SELF_CTRL:
        if(realData.step_count > 220)
        {
            realData.task_step = STEP_END;
        }
        realData.step_count++;
        break;
    case STEP_JIAOZHUN:
        if(realData.step_count > 100)
        {
            realData.step_count = 0;
            ptzControl(realData.channel, AppConfig::CamParm.angle_of_pitch_ptz);    //2、摄像头转动
            realData.channel++;
            if(realData.channel >= 16)
            {
                realData.channel = 0;
                AppConfig::is_calibration = false;
                realData.task_step = STEP_END;
            }
        }
        realData.step_count++;
        break;
    case STEP_END:
        realData.start_flag = false;
        realData.task_flag = false;
        realData.task_step = STEP_START;
        realData.step_count = 0;
        break;
    default:
        realData.task_step = STEP_ERROR;
        break;
    }
}
bool frmMain::get_mem_usage__()
{
    QProcess process;
    float free_rom = 0;
    process.start("free -m"); //使用free完成获取
    process.waitForFinished();
    process.readLine();
    QString str=process.readLine();
    str.replace("\n","");
    str.replace(QRegExp("( ){1,}")," ");//将连续空格替换为单个空格 用于分割
    auto lst = str.split(" ");if(lst.size() > 6)
    {
        qWarning("mem total:%.0lfMB free:%.0lfMB",lst[1].toDouble(),lst[3].toDouble());
        free_rom = lst[3].toDouble();
        if(free_rom < 100)
        {
            QProcess::execute(QString("sync"));//同步数据
            QProcess::execute(QString("sync"));//同步数据
            QProcess::execute(QString("sync"));//同步数据
            QProcess::execute( "/bin/sh -c \"echo 1 > /proc/sys/vm/drop_caches\"" );
            qWarning() << "内存过低，释放内存！";
        }

        return true;
    }
    return false;
}
bool frmMain::removeDirRecursively(const QString &dirPath)
{
    QDir dir(dirPath);
    if (!dir.exists()) {
        qWarning() << "Directory does not exist.";
        return false;
    }

    // 获取目录下所有的文件和子目录
    dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList fileList = dir.entryInfoList();

    // 删除所有文件
    foreach (const QFileInfo &fileInfo, fileList) {
        if (fileInfo.isDir()) {
            // 如果是子目录，递归调用删除
            if (!removeDirRecursively(fileInfo.absoluteFilePath())) {
                return false;
            }
        } else {
            // 如果是文件，删除文件
            QFile::remove(fileInfo.absoluteFilePath());
        }
    }

    // 删除空目录
    return dir.rmdir(dirPath);
}
/*
 * @功	能	日志清理函数
 * @参	数	文件路径，天数
 * */
void frmMain::cleanOldFiles(const QString &directoryPath, int maxDays = 30)
{
    // 创建 QDir 对象
    QDir dir(directoryPath);

    // 检查目录是否有效
    if (!dir.exists()) {
        qWarning() << "Directory does not exist:" << directoryPath;
        return;
    }

    // 获取目录下的所有文件和子目录
    QFileInfoList fileList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);

    // 获取当前时间
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 遍历文件列表
    foreach (const QFileInfo &fileInfo, fileList) {
        // 获取文件的最后修改时间
        QDateTime lastModified = fileInfo.lastModified();

        // 计算文件的存活天数
        int daysOld = lastModified.daysTo(currentDateTime);

        // 如果文件超过 maxDays（默认 30 天），则删除该文件
        if (daysOld > maxDays) {
            myWarning() << "Deleting file:" << fileInfo.filePath() << ", Age:" << daysOld << "days";

            // 删除文件
            if (QFile::remove(fileInfo.filePath())) {
                myWarning() << "File deleted successfully.";
            } else {
                qWarning() << "Failed to delete file.";
            }
        }
    }
}
/*
 * @功	能	次事件处理定时器
 * @参	数	无
 * */
void frmMain::Minor_ControlEvent()
{
    // 指定需要清理的目录路径
    QString directoryPath = "/home/baolong/log_run";
    static int check_count = 0;
    static int reboot_count = 0;
    static int error_count = 0;
    static int sync_count = 36000;
    static int intervalS = 0;
    if((AppConfig::NetworkError == true) || (AppConfig::ProfilesError == true))    //球机设备异常
    {
        if(error_count > 20)
        {
            error_count = 0;
            searchDevice(true);             //20s单播搜索设备
        }
        error_count ++;
    }else
    {
        error_count = 0;
    }
    if(AppConfig::NetworkError == false)
    {
        if(sync_count >= 60*60*10)
        {
            sync_count = 0;
            time_sync_slot();    //每10小时校准时间
        }
        sync_count++;
    }
    if(AppConfig::AlarmParm.is_send_realtime_value == true)//是否输出实时计算结果
    {
        real_backRequest();
    }
    if(AppConfig::is_update_code == true)
    {
        if(reboot_count > 5)
        {
            reboot_count = 0;
            AppConfig::is_update_code = false;
            if (removeDirRecursively("/home/baolong/Logs")) {
                myWarning() << "Directory and its contents deleted Logs successfully.";
            } else {
                qWarning() << "Failed to delete Logs the directory.";
            }
            if (removeDirRecursively("/home/baolong/Sqls")) {
                myWarning() << "Directory and its contents deleted Sqls successfully.";
            } else {
                qWarning() << "Failed to delete Sqls the directory.";
            }
            QProcess::execute(QString("reboot"));
        }
        reboot_count++;
    }
    if(check_count > 60*10)
    {
        check_count = 0;
        if(!get_mem_usage__())
        {
            qWarning() << "内存查询失败";
        }
    }
    if(intervalS >=600)//10分钟查询一次
    {
        intervalS = 0;
        cleanOldFiles(directoryPath, 30);// 清理 30 天前的文件
    }

    check_count++;
    intervalS++;
}
