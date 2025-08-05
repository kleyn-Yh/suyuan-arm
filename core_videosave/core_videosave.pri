#将当前目录加入到头文件路径
INCLUDEPATH += $$PWD
HEADERS += $$PWD/saveinclude.h

HEADERS += $$PWD/savehelper.h
SOURCES += $$PWD/savehelper.cpp

HEADERS += $$PWD/saveaudio.h
SOURCES += $$PWD/saveaudio.cpp

HEADERS += $$PWD/savevideo.h
SOURCES += $$PWD/savevideo.cpp
