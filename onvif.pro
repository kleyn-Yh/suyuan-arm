QT       += core gui
QT       += network xml
QT       += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += QT_MESSAGELOGCONTEXT

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    frmmain.cpp

HEADERS += \
    frmmain.h \
    head.h

FORMS +=

INCLUDEPATH += $$PWD
INCLUDEPATH += $$PWD/app

include ($$PWD/app/app.pri)
include ($$PWD/httpserver/httpserver.pri)
include ($$PWD/core_onvif/core_onvif.pri)

#DEFINES += ffmpeg
#contains(DEFINES, ffmpeg) {
#DEFINES += audiox videosave videoffmpeg
#include ($$PWD/core_base/core_base.pri)
#include ($$PWD/core_audio/core_audio.pri)
#include ($$PWD/core_video/core_video.pri)
#include ($$PWD/core_videobase/core_videobase.pri)
#include ($$PWD/core_videosave/core_videosave.pri)
#include ($$PWD/core_videoffmpeg/core_videoffmpeg.pri)
#include ($$PWD/core_videoopengl/core_videoopengl.pri)
#}
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



