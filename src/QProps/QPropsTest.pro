#-------------------------------------------------
#
# Project created by QtCreator 2016-09-01T23:30:32
#
#-------------------------------------------------

QT       += testlib gui

TARGET = QPropsTest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(QProps.pri)

HEADERS += \
    test/Example1.h

SOURCES += \
    test/QPropsTest.cpp
