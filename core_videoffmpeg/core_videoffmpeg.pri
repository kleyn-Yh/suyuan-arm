#将当前目录加入到头文件路径
INCLUDEPATH += $$PWD
include($$PWD/ffmpeginclude.pri)
HEADERS += $$PWD/ffmpeginclude.h
HEADERS += $$PWD/ffmpegstruct.h

HEADERS += $$PWD/ffmpeghelper.h
SOURCES += $$PWD/ffmpeghelper.cpp

HEADERS += $$PWD/ffmpegfilter.h
SOURCES += $$PWD/ffmpegfilter.cpp

HEADERS += $$PWD/ffmpegrun.h
SOURCES += $$PWD/ffmpegrun.cpp

HEADERS += $$PWD/ffmpegrunthread.h
SOURCES += $$PWD/ffmpegrunthread.cpp

contains(DEFINES, videosave) {
HEADERS += $$PWD/ffmpegsave.h
SOURCES += $$PWD/ffmpegsave.cpp
}

#当前组件功能较多可能被多个项目引入
#通过定义一个标识按照需要引入代码文件
contains(DEFINES, videoffmpeg) {
HEADERS += $$PWD/ffmpegsync.h
SOURCES += $$PWD/ffmpegsync.cpp

HEADERS += $$PWD/ffmpegthread.h
SOURCES += $$PWD/ffmpegthread.cpp
}
