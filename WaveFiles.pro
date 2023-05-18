#-------------------------------------------------
#
# Project created by QtCreator 2016-03-08T08:17:20
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WaveFiles
TEMPLATE = app


SOURCES += main.cpp\
        waveinspector.cpp \
    WaveFileHandler.cpp

HEADERS  += waveinspector.h \
    WaveFileHandler.h

FORMS    += waveinspector.ui
