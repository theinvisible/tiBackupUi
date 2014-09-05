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

#ifndef TIPREFERENCES_H
#define TIPREFERENCES_H

#include <QWidget>

#include "ticonf.h"

namespace Ui {
class tiPreferences;
}

class tiPreferences : public QWidget
{
    Q_OBJECT

public:
    explicit tiPreferences(QWidget *parent = 0);
    ~tiPreferences();

private slots:
    void on_btnAbort_clicked();

    void on_btnBackupjobs_clicked();

    void on_btnLogs_clicked();

    void on_btnSave_clicked();

    void on_cbSMTPAuth_toggled(bool checked);

private:
    Ui::tiPreferences *ui;

    tiConfMain *main_settings;
};

#endif // TIPREFERENCES_H
