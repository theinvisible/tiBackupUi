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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QLabel>

#include "tibackupadd.h"
#include "tibackupedit.h"
#include "tipreferences.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnAddBackup_clicked();
    void onjobAdded(tiBackupJob job);
    void onjobEdited(tiBackupJob job);

    void onActionAbout();
    void on_btnBackupJobDelete_clicked();
    void on_btnBackupJobEdit_clicked();
    void on_tvAvailBackupJobs_doubleClicked(const QModelIndex &index);

    void onActionPreferences();

    void on_btnStartManualBackup_clicked();

    void ontiBackupLogChanged(const QString & path);
    void onManualBackupFinished();

    void onTimeUpdate();

private:
    Ui::MainWindow *ui;
    QLabel *lblTime;

    void refreshBackupJobList();
};

#endif // MAINWINDOW_H
