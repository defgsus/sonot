#-------------------------------------------------
#
# Project created by QtCreator 2016-09-12T17:07:43
#
#-------------------------------------------------

QT       += widgets testlib

TARGET = QPropsTestGui
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app

DEFINES += SRCDIR=\\\"$$PWD/\\\"

include(QProps.pri)

HEADERS += \ 
    test/Example3.h

SOURCES += \
    test/QPropsTestGui.cpp
