#-------------------------------------------------
#
# Project created by QtCreator 2016-08-01T23:48:22
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TorClient
TEMPLATE = app

ICON = images/onion.png

SOURCES += main.cpp\
        TorClientWindow.cpp

HEADERS  += TorClientWindow.h \
    Constants.h

FORMS    += TorClientWindow.ui

RESOURCES += \
    images.qrc

DISTFILES +=
