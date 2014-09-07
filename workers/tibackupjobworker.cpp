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

#include "tibackupjobworker.h"

#include "ticonf.h"

tiBackupJobWorker::tiBackupJobWorker(QObject *parent) :
    QObject(parent)
{
}

void tiBackupJobWorker::setJobName(const QString &name)
{
    jobname = name;
}

void tiBackupJobWorker::process()
{
    // Work is here
    if(jobname.isEmpty())
    {
        emit finished();
        return;
    }

    tiConfBackupJobs jobss;
    tiBackupJob *workerJob = new tiBackupJob();
    *workerJob = *jobss.getJobByName(jobname);
    workerJob->startBackup();

    emit finished();
}
