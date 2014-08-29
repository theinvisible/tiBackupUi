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

#include "tipreferences.h"
#include "ui_tipreferences.h"

#include <QFileDialog>

tiPreferences::tiPreferences(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tiPreferences)
{
    ui->setupUi(this);

    main_settings = new tiConfMain;

    ui->cbDebug->setChecked(main_settings->getValue("main/debug").toBool());

    ui->leBackupjobsPath->setText(main_settings->getValue("paths/backupjobs").toString());
    ui->leLogsPath->setText(main_settings->getValue("paths/logs").toString());
}

tiPreferences::~tiPreferences()
{
    delete ui;
}

void tiPreferences::on_btnAbort_clicked()
{
    parentWidget()->close();
}

void tiPreferences::on_btnBackupjobs_clicked()
{
    QString startDir = (ui->leBackupjobsPath->text().isEmpty()) ? "/" : ui->leBackupjobsPath->text();

    QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Bitte wählen ein Verzeichnis"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leBackupjobsPath->setText(dir);
}

void tiPreferences::on_btnLogs_clicked()
{
    QString startDir = (ui->leLogsPath->text().isEmpty()) ? "/" : ui->leLogsPath->text();

    QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Bitte wählen ein Verzeichnis"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leLogsPath->setText(dir);
}

void tiPreferences::on_btnSave_clicked()
{
    main_settings->setValue("main/debug", ui->cbDebug->isChecked());

    main_settings->setValue("paths/backupjobs", ui->leBackupjobsPath->text());
    main_settings->setValue("paths/logs", ui->leLogsPath->text());

    main_settings->sync();

    parentWidget()->close();
}
