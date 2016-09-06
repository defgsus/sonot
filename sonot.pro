#-------------------------------------------------
#
# Project created by QtCreator 2016-08-10T13:44:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sonot
TEMPLATE = app
CONFIG += c++11

INCLUDEPATH += src

include(src/core/core.pri)
include(src/gui/gui.pri)

SOURCES += \
    src/main.cpp

DISTFILES += \
    README.md \
    .gitignore

