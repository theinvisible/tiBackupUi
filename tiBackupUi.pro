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
    tipreferences.cpp \
    tools/pbseditor.cpp \
    tools/pbsmanager.cpp \
    tools/scripteditor.cpp \
    tools/taskplaner.cpp

HEADERS  += mainwindow.h \
    tibackupadd.h \
    tipreferences.h \
    tools/pbseditor.h \
    tools/pbsmanager.h \
    tools/scripteditor.h \
    tools/taskplaner.h

FORMS    += mainwindow.ui \
    tibackupadd.ui \
    tipreferences.ui \
    tools/pbseditor.ui \
    tools/pbsmanager.ui \
    tools/scripteditor.ui \
    tools/taskplaner.ui

unix {
    message(Building for Unix)
    INCLUDEPATH += /usr/include/tibackuplib ../tiBackupLib
    LIBS += -L$$(HOME)/DEV/lib -ltiBackupLib
    QMAKE_CXXFLAGS_DEBUG += -pipe
    QMAKE_CXXFLAGS_RELEASE += -pipe -O2
}

RESOURCES += \
    resui.qrc

DISTFILES += \
    .gitlab-ci.yml \
    app-entry/tibackupui.desktop \
    debian/changelog \
    debian/control \
    debian/rules
