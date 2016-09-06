#-------------------------------------------------
#
# Project created by QtCreator 2016-09-01T23:30:32
#
#-------------------------------------------------

QT       += testlib gui widgets

TARGET = SonotGuiTest
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app


INCLUDEPATH += ./src

include(src/core/core.pri)
include(src/gui/gui.pri)

SOURCES += test/SonotGuiTest.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
