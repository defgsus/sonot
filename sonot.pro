#-------------------------------------------------
#
# Project created by QtCreator 2016-08-10T13:44:39
#
#-------------------------------------------------

QT       += core gui widgets multimedia

TARGET = sonot
TEMPLATE = app
CONFIG += c++11

DEFINES += SONOT_GUI

INCLUDEPATH += src

include(src/core.pri)
include(src/audio.pri)
include(src/gui.pri)
include(src/QProps.pri)

SOURCES += \
    src/main.cpp

DISTFILES += \
    README.md \
    .gitignore

