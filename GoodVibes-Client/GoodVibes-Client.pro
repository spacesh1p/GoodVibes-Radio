#-------------------------------------------------
#
# Project created by QtCreator 2017-03-27T18:24:19
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GoodVibes-Radio
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
    channelwidget.cpp \
    mainwindow.cpp \
    choosechannelwidget.cpp \
    channelsettingsdialog.cpp \
    playerwidget.cpp \
    channel.cpp \
    mediahandler.cpp \
    checkingpasswddialog.cpp \
    socket.cpp \
    socketthread.cpp \
    usernamesettingwidget.cpp

HEADERS  += \
    channelwidget.h \
    mainwindow.h \
    choosechannelwidget.h \
    channelsettingsdialog.h \
    playerwidget.h \
    channel.h \
    serverinfo.h \
    mediahandler.h \
    checkingpasswddialog.h \
    socket.h \
    socketthread.h \
    usernamesettingwidget.h

FORMS += \
    channelwidget.ui \
    mainwindow.ui \
    choosechannelwidget.ui \
    channelsettingsdialog.ui \
    playerwidget.ui \
    checkingpasswddialog.ui \
    usernamesettingwidget.ui

RESOURCES += \
    icons.qrc
