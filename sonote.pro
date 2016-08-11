#-------------------------------------------------
#
# Project created by QtCreator 2016-08-10T13:44:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sonote
TEMPLATE = app
CONFIG += c++11

INCLUDEPATH += src

SOURCES += \
    src/gui/MainWindow.cpp \
    src/main.cpp \
    src/gui/TextItem.cpp \
    src/io/JsonInterface.cpp \
    src/gui/ScoreView.cpp \
    src/gui/PageLayout.cpp

HEADERS  += \
    src/gui/MainWindow.h \
    src/gui/TextItem.h \
    src/io/JsonInterface.h \
    src/io/error.h \
    src/gui/ScoreView.h \
    src/gui/PageLayout.h

DISTFILES += \
    README.md \
    .gitignore
