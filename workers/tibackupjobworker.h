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

#ifndef TIBACKUPJOBWORKER_H
#define TIBACKUPJOBWORKER_H

#include <QObject>

class tiBackupJobWorker : public QObject
{
    Q_OBJECT
public:
    explicit tiBackupJobWorker(QObject *parent = 0);
    void setJobName(const QString &name);

signals:
    void finished();
    void error(QString err);

public slots:
    void process();

private:
    QString jobname;

};

#endif // TIBACKUPJOBWORKER_H
