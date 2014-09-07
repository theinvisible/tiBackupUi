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

void logMessageOutput(QtMsgType type, const char *msg)
{
    if(tibackupLog == 0)
    {

        tiConfMain main_settings;
        tibackupLog = new QFile(QString("%1/tibackup.log").arg(main_settings.getValue("paths/logs").toString()));
        tibackupLog->open(QIODevice::Append | QIODevice::Text);
    }

    QTextStream out(tibackupLog);
    QDateTime currentDate = QDateTime::currentDateTime();

    switch (type) {
    case QtDebugMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " Debug: " << msg << "\n";
        break;
    case QtWarningMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " Warning: " << msg << "\n";
        break;
    case QtCriticalMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " Critical: " << msg << "\n";
        break;
    case QtFatalMsg:
        out << currentDate.toString("MMM d hh:mm:ss").toStdString().c_str() << " Fatal: " << msg << "\n";
        tibackupLog->flush();
        abort();
    }

    tibackupLog->flush();
}

int main(int argc, char *argv[])
{
    qInstallMsgHandler(logMessageOutput);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
