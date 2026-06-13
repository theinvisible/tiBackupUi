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

#include "scripteditor.h"
#include "ui_scripteditor.h"

#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <QClipboard>
#include <QMessageBox>

#include "ipcclient.h"

scriptEditor::scriptEditor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::scriptEditor)
{
    ui->setupUi(this);
}

scriptEditor::~scriptEditor()
{
    delete ui;
}

void scriptEditor::loadScript(const QString &scriptPath)
{
    QFile script(scriptPath);
    ui->leScriptPath->setText(scriptPath);

    if(!script.exists())
        return;

    script.open(QIODevice::ReadOnly | QIODevice::Text);
    QString scriptSource = QString(script.readAll());
    ui->teScriptSource->setPlainText(scriptSource);
    script.close();
}

void scriptEditor::on_btnSave_clicked()
{
    // The GUI is unprivileged; the script lives under the (root-owned) scripts
    // directory and is written by the daemon over IPC.
    ipcClient::STATUS_ANSWER ret = ipcClient::instance()->saveScript(ui->leScriptPath->text(), ui->teScriptSource->toPlainText());
    if(ret.status != ipcClient::STATUS::STATUS_OK)
    {
        QMessageBox::warning(this, tr("Save script"),
                             tr("The script could not be saved. It must be located in the configured scripts directory and the tiBackup service must be running."));
        return;
    }

    parentWidget()->close();

    emit scriptSaved(ui->leScriptPath->text());
}

void scriptEditor::on_btnAbort_clicked()
{
    parentWidget()->close();
}

void scriptEditor::on_btnScriptFileChoose_clicked()
{
    QString startFile = (ui->leScriptPath->text().isEmpty()) ? "/" : ui->leScriptPath->text();

    QString file = QFileDialog::getOpenFileName(this, tr("Choose a file"), startFile);

    if(file.isEmpty())
        return;

    ui->leScriptPath->setText(file);
    loadScript(file);
}

void scriptEditor::on_btnInsertVar1_clicked()
{
    ui->teScriptSource->textCursor().insertText("%MNTBACKUPDIR%");
}

void scriptEditor::on_btnCopyVar1_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText("%MNTBACKUPDIR%");
}
