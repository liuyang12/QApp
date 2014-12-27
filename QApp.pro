#-------------------------------------------------
#
# Project created by QtCreator 2014-12-07T13:40:49
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QApp
TEMPLATE = app


SOURCES += main.cpp\
	login.cpp \
    tcplink.cpp \
    qappwindow.cpp \
    chatwindow.cpp

HEADERS  += \
	login.h \
    classConstant.h \
    tcplink.h \
    qappwindow.h \
    chatwindow.h

FORMS    += \
    login.ui \
    qappwindow.ui \
    chatwindow.ui

RESOURCES += \
    picture.qrc
