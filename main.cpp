#include "frmmain.h"

#include <QApplication>

/* 安装自定义打印消息的方法 */
void LogMsgOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex logMutex;
    logMutex.lock();
    QByteArray localMsg = msg.toLocal8Bit();
    QString log;
    QString timePoint = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    if (type != QtDebugMsg)
    {
        log.append(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss "));
        switch(type)
        {
            case QtInfoMsg:
                log.append(QString("(Info): %1  (FILE:%2:%3)").arg(localMsg.constData()).arg(context.file).arg(context.line));
                break;

            case QtWarningMsg:
                log.append(QString("(Warning): %1  (FILE:%2:%3)").arg(localMsg.constData()).arg(context.file).arg(context.line));
                break;

            case QtCriticalMsg:
                log.append(QString("(Critical): %1  (FILE:%2:%3, FUNCTION: %4)").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function));
                break;

            case QtFatalMsg:
                log.append(QString("(Fatal): %1  (FILE:%2:%3, FUNCTION: %4)").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function));
                break;
            default:
                break;
        }
        QDir tempDir;
        if (!tempDir.exists("/home/baolong/log_run"))
        {
            qDebug() << "log dir not exist" << endl;
            tempDir.mkpath("/home/baolong/log_run");
        }

        QFile file;
        QString path = QString("/home/baolong/log_run/%1.log").arg(timePoint);
        file.setFileName(path);

        if (!file.open(QIODevice::ReadWrite | QIODevice::Append))
        {
            //QString erinfo = file.errorString();
            qDebug() << "open file error" << endl;
        }
        QTextStream out(&file);
        out << log << "\n\r";
        file.flush();
        file.close();
    }
    // 如果是调试信息，可以选择同时打印到标准输出
    if (type == QtDebugMsg) {
        fprintf(stderr, "DEBUG: %s\n", msg.toLocal8Bit().data());
    }
    else if (type == QtInfoMsg)
    {
        fprintf(stderr, "Info: %s\n", msg.toLocal8Bit().data());
    }
    else if (type == QtWarningMsg)
    {
        fprintf(stderr, "Warning: %s\n", msg.toLocal8Bit().data());
    }
    logMutex.unlock();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    /* 安装自定义打印消息的方法 */
    qInstallMessageHandler(LogMsgOutput);

    frmMain w;
    w.show();
    return a.exec();
}
