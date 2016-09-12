#-------------------------------------------------
#
# Project created by QtCreator 2016-09-12T23:32:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PropertiesGui
TEMPLATE = app
CONFIG += c++11

INCLUDEPATH += ../../..

HEADERS += \
    MainWindow.h \
    ../../error.h \
    ../../JsonInterface.h \
    ../../JsonInterfaceHelper.h \
    ../../Properties.h \
    ../../PropertiesView.h \
    ../../PropertyWidget.h \
    ../../qprops_global.h

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    ../../JsonInterface.cpp \
    ../../JsonInterfaceHelper.cpp \
    ../../Properties.cpp \
    ../../Properties_json.cpp \
    ../../Properties_qvariant.cpp \
    ../../PropertiesView.cpp \
    ../../PropertyWidget.cpp

