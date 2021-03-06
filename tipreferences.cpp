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
    ui->leBackupScriptsPath->setText(main_settings->getValue("paths/scripts").toString());
    ui->leInitdPath->setText(main_settings->getValue("paths/initd").toString());

    ui->leSMTPServer->setText(main_settings->getValue("smtp/server").toString());
    ui->cbSMTPAuth->setChecked(main_settings->getValue("smtp/auth").toBool());
    ui->leSMTPUsername->setText(main_settings->getValue("smtp/username").toString());
    ui->leSMTPPassword->setText(QByteArray().fromBase64(QByteArray().append(main_settings->getValue("smtp/password").toString())));

    if(ui->cbSMTPAuth->isChecked() == true)
    {
        ui->leSMTPUsername->setEnabled(true);
        ui->leSMTPPassword->setEnabled(true);
    }
    else
    {
        ui->leSMTPUsername->setDisabled(true);
        ui->leSMTPPassword->setDisabled(true);
    }
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

    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty())
        return;

    ui->leBackupjobsPath->setText(dir);
}

void tiPreferences::on_btnLogs_clicked()
{
    QString startDir = (ui->leLogsPath->text().isEmpty()) ? "/" : ui->leLogsPath->text();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty())
        return;

    ui->leLogsPath->setText(dir);
}

void tiPreferences::on_btnSave_clicked()
{
    main_settings->setValue("main/debug", ui->cbDebug->isChecked());

    main_settings->setValue("paths/backupjobs", ui->leBackupjobsPath->text());
    main_settings->setValue("paths/logs", ui->leLogsPath->text());
    main_settings->setValue("paths/scripts", ui->leBackupScriptsPath->text());
    main_settings->setValue("paths/initd", ui->leInitdPath->text());

    main_settings->setValue("smtp/server", ui->leSMTPServer->text());
    main_settings->setValue("smtp/auth", ui->cbSMTPAuth->isChecked());
    main_settings->setValue("smtp/username", ui->leSMTPUsername->text());
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    main_settings->setValue("smtp/password", QString(ui->leSMTPPassword->text().toLatin1().toBase64()));
#else
    main_settings->setValue("smtp/password", QString(ui->leSMTPPassword->text().toAscii().toBase64()));
#endif

    main_settings->sync();

    parentWidget()->close();
}

void tiPreferences::on_cbSMTPAuth_toggled(bool checked)
{
    if(checked == true)
    {
        ui->leSMTPUsername->setEnabled(true);
        ui->leSMTPPassword->setEnabled(true);
    }
    else
    {
        ui->leSMTPUsername->setDisabled(true);
        ui->leSMTPPassword->setDisabled(true);
    }
}

void tiPreferences::on_btnBackupjobScripts_clicked()
{
    QString startDir = (ui->leBackupScriptsPath->text().isEmpty()) ? "/" : ui->leBackupScriptsPath->text();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty())
        return;

    ui->leBackupScriptsPath->setText(dir);
}

void tiPreferences::on_btnInitd_clicked()
{
    QString startDir = (ui->leInitdPath->text().isEmpty()) ? "/" : ui->leInitdPath->text();

    QString file = QFileDialog::getOpenFileName(this, tr("Choose the init.d file"), startDir);

    if(file.isEmpty())
        return;

    ui->leInitdPath->setText(file);
}
