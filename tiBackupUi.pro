#-------------------------------------------------
#
# Project created by QtCreator 2014-08-23T15:12:02
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tiBackupUi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tibackupadd.cpp \
    tibackupedit.cpp \
    tipreferences.cpp \
    tools/scripteditor.cpp \
    tools/taskplaner.cpp

HEADERS  += mainwindow.h \
    tibackupadd.h \
    tibackupedit.h \
    tipreferences.h \
    tools/scripteditor.h \
    tools/taskplaner.h

FORMS    += mainwindow.ui \
    tibackupadd.ui \
    tibackupedit.ui \
    tipreferences.ui \
    tools/scripteditor.ui \
    tools/taskplaner.ui

unix {
    message(Building for Unix)
    INCLUDEPATH += /usr/include/tibackuplib
    LIBS += -ltiBackupLib
    QMAKE_CXXFLAGS_DEBUG += -pipe
    QMAKE_CXXFLAGS_RELEASE += -pipe -O2
}

RESOURCES += \
    resui.qrc
