/*
 *
tiBackupUi - A intelligent desktop/standalone backup system user interface

Copyright (C) 2014 Rene Hadler, rene@hadler.me, https://hadler.me

    This file is part of tiBackup.

    tiBackupUi is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    tiBackupUi is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with tiBackupUi.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

QFile *tibackupLog = 0;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
void logMessageOutput(QtMsgType type, const QMessageLogContext &, const QString & str)
{
    const char * msg = str.toStdString().c_str();
#else
void logMessageOutput(QtMsgType type, const char *msg)
{
#endif
    tiConfMain main_settings;

    if(tibackupLog == 0)
    {
        tibackupLog = new QFile(QString("%1/tibackup.log").arg(main_settings.getValue("paths/logs").toString()));
        tibackupLog->open(QIODevice::Append | QIODevice::Text);
    }

    bool tidebug = main_settings.getValue("main/debug").toBool();

    QTextStream out(tibackupLog);
    QDateTime currentDate = QDateTime::currentDateTime();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    switch (type) {
    case QtDebugMsg:
        if(tidebug == true)
            out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Debug: " << str << "\n";
        break;
    case QtInfoMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Info: " << str << "\n";
        break;
    case QtWarningMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Warning: " << str << "\n";
        break;
    case QtCriticalMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Critical: " << str << "\n";
        break;
    case QtFatalMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Fatal: " << str << "\n";
        tibackupLog->flush();
        abort();
    }
#else
    switch (type) {
    case QtDebugMsg:
        if(tidebug == true)
            out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Debug: " << QString::fromUtf8(msg).toStdString().c_str() << "\n";
        break;
    case QtWarningMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Warning: " << QString::fromUtf8(msg).toStdString().c_str() << "\n";
        break;
    case QtCriticalMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Critical: " << QString::fromUtf8(msg).toStdString().c_str() << "\n";
        break;
    case QtFatalMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Fatal: " << QString::fromUtf8(msg).toStdString().c_str() << "\n";
        tibackupLog->flush();
        abort();
    }
#endif

    tibackupLog->flush();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logMessageOutput);

    qRegisterMetaType<DeviceDiskPartition>("DeviceDiskPartition");

    QApplication a(argc, argv);

    Q_INIT_RESOURCE(resdata);

    MainWindow w;
    w.show();

    return a.exec();
}
