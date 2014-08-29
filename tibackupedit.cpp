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

#include "tibackupedit.h"
#include "ui_tibackupedit.h"

#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMessageBox>

tiBackupEdit::tiBackupEdit(QWidget *parent, tiBackupJob *job) :
    QWidget(parent),
    ui(new Ui::tiBackupEdit)
{
    ui->setupUi(this);
    currentJob = job;

    parent->installEventFilter(this);

    QStringList headers;
    headers << "Quellordner" << "Zielordner";

    QStandardItemModel *model = new QStandardItemModel(ui->tvBackupFolders);
    model->setHorizontalHeaderLabels(headers);

    ui->tvBackupFolders->setModel(model);

    updateJobDetails();
}

tiBackupEdit::~tiBackupEdit()
{
    delete ui;
    delete currentJob;
}

void tiBackupEdit::updateJobDetails()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());
    model->removeRows(0, model->rowCount());

    ui->leBackupJobName->setText(currentJob->name);
    ui->leBackupPartition->setText(currentJob->partition_uuid);

    QHashIterator<QString, QString> it(currentJob->backupdirs);
    QStandardItem *item = 0;
    QStandardItem *item2 = 0;
    int row = model->rowCount();

    while(it.hasNext())
    {
        it.next();

        item = new QStandardItem(it.key());
        item2 = new QStandardItem(it.value());

        model->setItem(row, 0, item);
        model->setItem(row, 1, item2);

        row++;
    }

    ui->cbDeleteAddFilesOnDest->setChecked(currentJob->delete_add_file_on_dest);
    ui->cbBackupOnHotplug->setChecked(currentJob->start_backup_on_hotplug);
    ui->cbSaveLog->setChecked(currentJob->save_log);
}

bool tiBackupEdit::eventFilter(QObject *object, QEvent *event)
{
    if(object == parentWidget() && event->type() == QEvent::Close)
    {
        /*
        int ret = QMessageBox::warning(this, QString::fromUtf8("Fenster schließen"),
                                    QString::fromUtf8("Alle Änderungen gehen verloren. Fortfahren?"),
                                    QMessageBox::Yes | QMessageBox::No);

        switch(ret)
        {
        case QMessageBox::Yes:
            return false;
        case QMessageBox::No:
        default:
            event->ignore();
            return true;
        }
        */

        return false;
    }

    return false;
}

void tiBackupEdit::on_btnSelectSource_clicked()
{
    QString startDir = (ui->leSourceFolder->text().isEmpty()) ? "/" : ui->leSourceFolder->text();

    QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Bitte wählen Sie das Quellverzeichnis"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leSourceFolder->setText(dir);
}

void tiBackupEdit::on_btnSelectDest_clicked()
{
    QString startDir = (ui->leDestFolder->text().isEmpty()) ? "/" : ui->leDestFolder->text();

    QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Bitte wählen Sie das Zielverzeichnis"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leDestFolder->setText(dir);
}

void tiBackupEdit::on_btnAddBackupFolder_clicked()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());

    QStandardItem *item = new QStandardItem(ui->leSourceFolder->text());
    QStandardItem *item2 = new QStandardItem(ui->leDestFolder->text());

    int row = model->rowCount();
    model->setItem(row, 0, item);
    model->setItem(row, 1, item2);

    qDebug() << "rowcount::" << model->rowCount();
}

void tiBackupEdit::on_btnRemoveBackupFolder_clicked()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());
    QItemSelectionModel *selmodel = ui->tvBackupFolders->selectionModel();
    //ui->tvBackupFolders->selectedItems();

    QModelIndexList sellist = selmodel->selectedRows();
    if(sellist.count() >= 1)
    {
        QStandardItem *selitem = model->itemFromIndex(sellist.at(0));
        model->removeRow(selitem->row());
    }
}

void tiBackupEdit::on_btnCancel_clicked()
{
    parentWidget()->close();
}

void tiBackupEdit::on_btnEditBackupJob_clicked()
{
    //QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());
    tiConfBackupJobs jobs;

    if(ui->leBackupJobName->text().isEmpty())
    {
        QMessageBox::information(this, QString::fromUtf8("Backupjob hinzufügen"), QString::fromUtf8("Es muss ein Name für den Backupjob angegeben werden."));
        return;
    }

    tiBackupJob job = *currentJob;

    if(currentJob->name != ui->leBackupJobName->text())
    {
        // We must rename the job
        if(!jobs.renameJob(currentJob->name, ui->leBackupJobName->text()))
        {
            QMessageBox::information(this, QString::fromUtf8("Backupjob hinzufügen"), QString::fromUtf8("Der Backupjob-Name konnte nicht geändert werden."));
            return;
        }
        job.name = ui->leBackupJobName->text();
    }

    //job.device = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    job.partition_uuid = ui->leBackupPartition->text();
    job.delete_add_file_on_dest = ui->cbDeleteAddFilesOnDest->isChecked();
    job.start_backup_on_hotplug = ui->cbBackupOnHotplug->isChecked();
    job.save_log = ui->cbSaveLog->isChecked();

    /*
    DeviceDisk selDisk;
    selDisk.devname = devname;

    DeviceDiskPartition part = selDisk.getPartitionByUUID(job.partition_uuid);
    TiBackupLib lib;
    */

    QHash<QString, QString> h;
    for(int i=0; i < model->rowCount(); i++)
    {
        //h.insertMulti(model->item(i, 0)->text(), TiBackupLib::convertPath2Generic(model->item(i, 1)->text(), lib.getMountDir(part.name)));
        h.insertMulti(model->item(i, 0)->text(), model->item(i, 1)->text());
    }
    job.backupdirs = h;

    jobs.saveBackupJob(job);

    parentWidget()->close();

    emit jobEdited(job);
}
