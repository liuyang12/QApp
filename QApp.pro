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
		qwindow.cpp \
	login.cpp

HEADERS  += qwindow.h \
	login.h \
    ClassConstant.h

FORMS    += qwindow.ui \
    login.ui
