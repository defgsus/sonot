#-------------------------------------------------
#
# Project created by QtCreator 2016-09-01T23:30:32
#
#-------------------------------------------------

QT       += testlib gui

TARGET = SonotCoreTest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app


INCLUDEPATH += ./src

include(src/core/core.pri)

SOURCES += test/SonotCoreTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
