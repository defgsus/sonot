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

SOURCES += \
    src/gui/MainWindow.cpp \
    src/main.cpp \
    src/gui/TextItem.cpp \
    src/io/JsonInterface.cpp \
    src/gui/ScoreView.cpp \
    src/gui/PageLayout.cpp \
    src/gui/PageAnnotation.cpp \
    src/gui/PageAnnotationTemplate.cpp \
    src/gui/PageSize.cpp \
    src/gui/ScoreLayout.cpp

HEADERS  += \
    src/gui/MainWindow.h \
    src/gui/TextItem.h \
    src/io/JsonInterface.h \
    src/io/error.h \
    src/gui/ScoreView.h \
    src/gui/PageLayout.h \
    src/gui/PageAnnotation.h \
    src/gui/PageAnnotationTemplate.h \
    src/gui/PageSize.h \
    src/gui/ScoreLayout.h

DISTFILES += \
    README.md \
    .gitignore
