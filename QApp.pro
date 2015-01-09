#-------------------------------------------------
#
# Project created by QtCreator 2014-12-07T13:40:49
#
#-------------------------------------------------

QT       += core gui network sql
QT       += multimedia widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QApp
TEMPLATE = app


SOURCES += main.cpp\
	login.cpp \
    tcplink.cpp \
    qappwindow.cpp \
    chatwindow.cpp \
    friendinfo.cpp \
    messdialog.cpp \
    video.cpp

HEADERS  += \
	login.h \
    classConstant.h \
    tcplink.h \
    qappwindow.h \
    chatwindow.h \
    friendinfo.h \
    messdialog.h \
    video.h

FORMS    += \
    login.ui \
    qappwindow.ui \
    chatwindow.ui \
    friendinfo.ui \
    messdialog.ui \
    video.ui

RESOURCES += \
    picture.qrc \
    mainpicture.qrc \
    chatwindow.qrc \
    friendinfo.qrc

RC_FILE = myicon.rc

#USED BY GDX
#INCLUDEPATH += C:\OpenCV\bin\install\include  \
#C:\OpenCV\bin\install\include\opencv  \
#C:\OpenCV\bin\install\include\opencv2
#LIBS += -L"C:\OpenCV\bin\install\x64\mingw\bin"
#LIBS += -lopencv_core249 -lopencv_highgui249 -lopencv_imgproc249 -lopencv_features2d249
#-lopencv_calib3d249



#USED BY YOUNG
INCLUDEPATH += D:\opencv\build\include\opencv\
               D:\opencv\build\include\opencv2\
               D:\opencv\build\include

LIBS += D:\opencv\MinGW\lib\libopencv_calib3d249.dll.a\
        D:\opencv\MinGW\lib\libopencv_contrib249.dll.a\
        D:\opencv\MinGW\lib\libopencv_core249.dll.a\
        D:\opencv\MinGW\lib\libopencv_features2d249.dll.a\
        D:\opencv\MinGW\lib\libopencv_flann249.dll.a\
        D:\opencv\MinGW\lib\libopencv_gpu249.dll.a\
        D:\opencv\MinGW\lib\libopencv_highgui249.dll.a\
        D:\opencv\MinGW\lib\libopencv_imgproc249.dll.a\
        D:\opencv\MinGW\lib\libopencv_legacy249.dll.a\
        D:\opencv\MinGW\lib\libopencv_ml249.dll.a\
        D:\opencv\MinGW\lib\libopencv_objdetect249.dll.a\
        D:\opencv\MinGW\lib\libopencv_video249.dll.a



#USED BY WEIJIE
#INCLUDEPATH += D:\openCV\opencv\bin\install\include  \
#D:\openCV\opencv\bin\install\include\opencv  \
#D:\openCV\opencv\bin\install\include\opencv2
#LIBS += -L"D:\openCV\opencv\bin\install\x64\mingw\bin"
#LIBS += -lopencv_core249 -lopencv_highgui249 -lopencv_imgproc249
