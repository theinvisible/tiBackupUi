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
    QFile scriptFile(ui->leScriptPath->text());
    scriptFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&scriptFile);
    out << ui->teScriptSource->toPlainText();
    scriptFile.close();
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

    QString file = QFileDialog::getOpenFileName(this, trUtf8("Bitte wähle eine Datei"), startFile);

    if(file.isEmpty())
        return;

    ui->leScriptPath->setText(file);
    loadScript(file);
}
