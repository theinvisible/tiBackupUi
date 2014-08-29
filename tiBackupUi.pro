#-------------------------------------------------
#
# Project created by QtCreator 2014-08-23T15:12:02
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tiBackupUi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tibackupadd.cpp \
    tibackupedit.cpp \
    tipreferences.cpp

HEADERS  += mainwindow.h \
    tibackupadd.h \
    tibackupedit.h \
    tipreferences.h

FORMS    += mainwindow.ui \
    tibackupadd.ui \
    tibackupedit.ui \
    tipreferences.ui

unix {
    message(Building for Unix)
    INCLUDEPATH += /home/rene/DEV/qtcreator/tiBackupLib
    LIBS += -L/home/rene/DEV/lib -ltiBackupLib
    QMAKE_CXXFLAGS_DEBUG += -pipe
    QMAKE_CXXFLAGS_RELEASE += -march=native \
        -pipe \
        -fomit-frame-pointer
}
