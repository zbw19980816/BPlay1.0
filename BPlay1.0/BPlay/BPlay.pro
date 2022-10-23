QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Baudio.cpp \
    Bbutton.cpp \
    Bffmpeg.cpp \
    Bpublic.cpp \
    Bvideo.cpp \
    Bwidget.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Baudio.h \
    Bbutton.h \
    Bffmpeg.h \
    Bpublic.h \
    Bvideo.h \
    Bwidget.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD/ffmpeg/include
LIBS  +=-L$$PWD/ffmpeg/lib
LIBS  += -lavcodec -lavformat -lswscale -lavutil -lswresample

RESOURCES += \
    Image.qrc
