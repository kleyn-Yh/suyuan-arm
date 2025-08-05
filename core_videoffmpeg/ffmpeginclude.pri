#如果没有定义任何版本则默认是ffmpeg4
#如果要支持xp只能用ffmpeg2/ffmpeg3
!contains(DEFINES, ffmpeg2) {
!contains(DEFINES, ffmpeg3) {
!contains(DEFINES, ffmpeg4) {
!contains(DEFINES, ffmpeg5) {
DEFINES += ffmpeg4
}}}}

#区分版本
contains(DEFINES, ffmpeg5) {
strPath = ffmpeg5
} else {
contains(DEFINES, ffmpeg4) {
strPath = ffmpeg4
} else {
contains(DEFINES, ffmpeg3) {
strPath = ffmpeg3
} else {
strPath = ffmpeg2
}}}

#区分系统
win32 {
contains(QT_ARCH, x86_64) {
strLib = winlib64
} else {
strLib = winlib
}}

#unix系统(包括linux/mac/android等)
unix {
contains(QT_ARCH, x86_64) {
strLib = linuxlib64
} else {
strLib = linuxlib
}}

#mac系统目前都是64位的
macx {
strLib = maclib64
}

#android系统(armeabi-v7a/arm64-v8a)
android {
contains(QT_ARCH, arm64-v8a) {
strLib = androidlib64
} else {
strLib = androidlib
}}

#包含头文件
INCLUDEPATH += $$PWD/$$strPath/include

#在某些环境下可能如果报错的话可能还需要主动链接一些库 -lm -lz -lbz2 -lrt -ld -llzma -lvdpau -lX11 -lx264 等
#具体报错提示自行搜索可以找到答案(增加需要链接的库即可)
#链接库文件(编译好以后需要将动态库文件复制到可执行文件下的lib目录才能正常运行)
LIBS += -L$$PWD/$$strPath/$$strLib/ -lavformat -lavcodec -lavfilter -lswscale -lavutil -lswresample

#如果系统环境变量中能够找到库则可以用下面的方式(uos系统环境中有ffmpeg库)
#LIBS += -L/ -lavformat -lavcodec -lavfilter -lswscale -lavutil -lswresample
#LIBS += -L$$PWD/armlib/ -lavformat -lavcodec -lavfilter -lswscale -lavutil -lswresample
#LIBS += -L$$PWD/armlib4.5/ -lavformat -lavcodec -lavfilter -lswscale -lavutil -lswresample -lpostproc

#引入本地设备(linux上如果是静态的ffmpeg库则需要去掉)
DEFINES += ffmpegdevice

android {
QMAKE_LFLAGS += -lOpenSLES
DEFINES -= ffmpegdevice
LIBS += -L$$PWD/$$strPath/$$strLib/ -lx264
#默认提供的是ffmpeg4对应的动态库(如果是静态库则不用下面这些打包)
ANDROID_EXTRA_LIBS += $$PWD/$$strPath/$$strLib/libavcodec.so
ANDROID_EXTRA_LIBS += $$PWD/$$strPath/$$strLib/libavfilter.so
ANDROID_EXTRA_LIBS += $$PWD/$$strPath/$$strLib/libavformat.so
ANDROID_EXTRA_LIBS += $$PWD/$$strPath/$$strLib/libavutil.so
ANDROID_EXTRA_LIBS += $$PWD/$$strPath/$$strLib/libswresample.so
ANDROID_EXTRA_LIBS += $$PWD/$$strPath/$$strLib/libswscale.so
ANDROID_EXTRA_LIBS += $$PWD/$$strPath/$$strLib/libfdk-aac.so
ANDROID_EXTRA_LIBS += $$PWD/$$strPath/$$strLib/libx264.so
}

#引入本地设备库
contains(DEFINES, ffmpegdevice) {
LIBS += -L$$PWD/$$strPath/$$strLib/ -lavdevice
}

#指定从可执行文件所在目录查找动态库
#查看rpath命令 readelf -d xxx | grep 'RPATH'
linux:!android {
LIBS += -L$$PWD/$$strPath/$$strLib/ -lpostproc -lx264 -lx265
QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN\'"
QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/lib\'"
QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/../lib\'"
}
