#-------------------------------------------------
#
# Project created by QtCreator 2016-09-09T11:29:17
#
#-------------------------------------------------

QT       += gui widgets

TARGET = QProps
TEMPLATE = lib
CONFIG += c++11

CONFIG += staticlib

DEFINES += QPROPS_LIBRARY

include(QProps.pri)

unix {
    target.path = /usr/lib
    INSTALLS += target
}

