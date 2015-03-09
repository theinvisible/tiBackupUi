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

#ifndef TIBACKUPADD_H
#define TIBACKUPADD_H

#include <QWidget>

#include "ticonf.h"
#include "tibackuplib.h"

namespace Ui {
class tiBackupAdd;
}

class tiBackupAdd : public QWidget
{
    Q_OBJECT

public:
    explicit tiBackupAdd(QWidget *parent = 0);
    ~tiBackupAdd();

private slots:
    void on_comboBackupDevice_currentIndexChanged(int index);
    void on_comboBackupPartition_currentIndexChanged(int index);
    void on_btnSelectSource_clicked();
    void on_btnSelectDest_clicked();
    void on_btnAddBackupFolder_clicked();
    void on_btnRemoveBackupFolder_clicked();
    void on_btnAddBackupJob_clicked();
    void on_btnPartitionMount_clicked();
    void on_btnCancel_clicked();

    void on_btnEditScriptBeforeBackup_clicked();
    void on_scriptBefore_changed(QString scriptPath);

    void on_btnEditScriptAfterBackup_clicked();
    void on_scriptAfter_changed(QString scriptPath);

signals:
    void jobAdded(tiBackupJob job);

private:
    Ui::tiBackupAdd *ui;

    void updatePartitionInformation();

protected:
    bool eventFilter(QObject *object, QEvent *event);
};

#endif // TIBACKUPADD_H
