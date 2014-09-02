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
    ui->btnPartitionMount->setDisabled(true);

    parent->installEventFilter(this);

    QStringList headers;
    headers << "Quellordner" << "Zielordner";

    QStandardItemModel *model = new QStandardItemModel(ui->tvBackupFolders);
    model->setHorizontalHeaderLabels(headers);

    ui->tvBackupFolders->setModel(model);
    ui->tvBackupFolders->header()->resizeSection(0, 350);

    // Load available Backup devices
    TiBackupLib blib;
    QList<DeviceDisk> disks = blib.getAttachedDisks();
    qDebug() << "disks found:" << disks.count();

    for(int i=0; i < disks.count(); i++)
    {
        DeviceDisk disk = disks.at(i);

        qDebug() << "disk:" << disk.devname;
        if(disk.devtype == "disk")
        {
            ui->comboBackupDevice->insertItem(0, QString("%1 - %2 (%3)").arg(disk.vendor, disk.model, disk.devname), disk.devname);
        }
    }

    updateJobDetails();
    updatePartitionInformation();
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
    ui->comboBackupDevice->setEditText(currentJob->device);
    ui->comboBackupPartition->setEditText(currentJob->partition_uuid);

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

    // We must see if the current job disk is attached
    // Load available Backup devices
    TiBackupLib blib;
    QList<DeviceDisk> disks = blib.getAttachedDisks();
    for(int i=0; i < disks.count(); i++)
    {
        DeviceDisk disk = disks.at(i);

        qDebug() << "disk:" << disk.devname;
        if(disk.devtype == "disk")
        {
            disk.readPartitions();
            for(int j=0; j < disk.partitions.count(); j++)
            {
                DeviceDiskPartition part = disk.partitions.at(j);

                if(part.uuid.isEmpty())
                    continue;

                if(part.uuid == currentJob->partition_uuid)
                {
                    // Job disk is attached right now
                    qDebug() << "job disk is attached right now";

                    int devrow = ui->comboBackupDevice->findData(disk.devname);
                    int partrow = ui->comboBackupPartition->findData(part.uuid);
                    qDebug() << "devrow::" << devrow << "::partrow::" << partrow;

                    ui->comboBackupDevice->setCurrentIndex(devrow);
                    ui->comboBackupPartition->setCurrentIndex(partrow);

                    return;
                }
            }
        }
    }
}

void tiBackupEdit::updatePartitionInformation()
{
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QString uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    qDebug() << "tiBackupEdit::updatePartitionInformation() -> devname:" << devname;
    qDebug() << "tiBackupEdit::updatePartitionInformation() -> uuid:" << uuid;

    DeviceDisk selDisk;
    selDisk.devname = devname;

    DeviceDiskPartition part = selDisk.getPartitionByUUID(uuid);
    ui->lblDriveType->setText(part.type);

    TiBackupLib lib;
    if(lib.isMounted(part.name))
    {
        ui->btnPartitionMount->setDisabled(true);
        ui->lblMountInfo->setText(QString("Partition %1 ist gemounted auf %2").arg(part.name, lib.getMountDir(part.name)));
    }
    else
    {
        ui->btnPartitionMount->setEnabled(true);
        ui->lblMountInfo->setText(QString("Partition %1 ist nicht gemounted").arg(part.name));
    }
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
    //job.partition_uuid = ui->leBackupPartition->text();
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

void tiBackupEdit::on_comboBackupDevice_currentIndexChanged(int index)
{
    // Load available Backup partitions
    QString devname = ui->comboBackupDevice->itemData(index).toString();
    ui->comboBackupPartition->clear();

    qDebug() << "tiBackupEdit::on_comboBackupDevice_currentIndexChanged(int index) devname:" << devname;
    DeviceDisk selDisk;
    selDisk.devname = devname;
    selDisk.readPartitions();
    for(int i=0; i < selDisk.partitions.count(); i++)
    {
        DeviceDiskPartition part = selDisk.partitions.at(i);

        if(part.uuid.isEmpty())
            continue;

        ui->comboBackupPartition->insertItem(0, QString("%1 (%2)").arg(part.name, part.uuid), part.uuid);
    }
}

void tiBackupEdit::on_comboBackupPartition_currentIndexChanged(int index)
{
    updatePartitionInformation();
}

void tiBackupEdit::on_btnPartitionMount_clicked()
{
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QString uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    qDebug() << "selected part uuid:" << uuid;
    DeviceDisk selDisk;
    selDisk.devname = devname;

    DeviceDiskPartition part = selDisk.getPartitionByUUID(uuid);

    TiBackupLib lib;
    if(lib.isMounted(part.name))
    {
        QMessageBox::information(this, QString::fromUtf8("Mountinformation"), QString::fromUtf8("Das Laufwerk ist schon gemounted."));
    }
    else
    {
        lib.mountPartition(&part);
        updatePartitionInformation();
    }
}
