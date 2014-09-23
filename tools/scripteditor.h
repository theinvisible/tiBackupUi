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

#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QWidget>

namespace Ui {
class scriptEditor;
}

class scriptEditor : public QWidget
{
    Q_OBJECT

public:
    explicit scriptEditor(QWidget *parent = 0);
    ~scriptEditor();

    void loadScript(const QString &scriptPath);

private slots:
    void on_btnSave_clicked();

    void on_btnAbort_clicked();

    void on_btnScriptFileChoose_clicked();

private:
    Ui::scriptEditor *ui;
};

#endif // SCRIPTEDITOR_H
