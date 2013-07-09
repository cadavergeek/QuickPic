#-------------------------------------------------
#
# Project created by QtCreator 2013-07-08T10:29:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QuickPic
TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp

HEADERS  += MainWindow.h

FORMS    += MainWindow.ui

RESOURCES += \
    MainWindow.qrc

CONFIG += c++11

mac {
  QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++ -mmacosx-version-min=10.7
  LIBS += -stdlib=libc++ -mmacosx-version-min=10.7
}
