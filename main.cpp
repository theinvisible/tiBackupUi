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
#include <QStandardPaths>
#include <QDir>

#include "config.h"

QFile *tibackupLog = 0;

void logMessageOutput(QtMsgType type, const QMessageLogContext &, const QString & str)
{
    QTextStream sout(stdout);

    if(tibackupLog == 0)
    {
        // The GUI runs unprivileged: write our own log to a user-writable location
        // (the daemon owns /etc/tibackup/logs/tibackup.log).
        QString logDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/tiBackupUi";
        QDir().mkpath(logDir);
        tibackupLog = new QFile(QString("%1/tibackupui.log").arg(logDir));
        tibackupLog->open(QIODevice::Append | QIODevice::Text);
    }

    // main.conf is read-only for us and may not exist before the daemon initialises it.
    bool tidebug = false;
    if(QFile::exists(tibackup_config::file_main))
    {
        tiConfMain main_settings;
        tidebug = main_settings.getValue("main/debug").toBool();
    }

    QTextStream out(tibackupLog);
    QDateTime currentDate = QDateTime::currentDateTime();

    sout << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Info: " << str << "\n";

    switch (type) {
    case QtDebugMsg:
        if(tidebug == true)
            out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Debug: " << str << "\n";
        break;
    case QtInfoMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " tiBackupUi::Info: " << str << "\n";
        sout << str;
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

    tibackupLog->flush();
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logMessageOutput);

    qRegisterMetaType<DeviceDiskPartition>("DeviceDiskPartition");

    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    return a.exec();
}
