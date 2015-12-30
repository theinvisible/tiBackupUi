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

#include "tibackupadd.h"
#include "ui_tibackupadd.h"

#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QMainWindow>
#include <QDateTime>

#include "tools/scripteditor.h"

tiBackupAdd::tiBackupAdd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tiBackupAdd)
{
    ui->setupUi(this);

    int i=0;
    ui->btnPartitionMount->setDisabled(true);

    // Load available Backup devices
    TiBackupLib blib;
    QList<DeviceDisk> disks = blib.getAttachedDisks();
    qDebug() << "tiBackupAdd::tiBackupAdd() -> disks found:" << disks.count();

    for(i=0; i < disks.count(); i++)
    {
        DeviceDisk disk = disks.at(i);

        qDebug() << "tiBackupAdd::tiBackupAdd() -> disk:" << disk.devname;
        if(disk.devtype == "disk")
        {
            ui->comboBackupDevice->insertItem(0, QString("%1 - %2 (%3)").arg(disk.vendor, disk.model, disk.devname), disk.devname);
        }
    }

    parent->installEventFilter(this);

    QStringList headers;
    headers << "Quellordner" << "Zielordner";

    QStandardItemModel *model = new QStandardItemModel(ui->tvBackupFolders);
    model->setHorizontalHeaderLabels(headers);

    ui->tvBackupFolders->setModel(model);
    ui->tvBackupFolders->header()->resizeSection(0, 350);
}

tiBackupAdd::~tiBackupAdd()
{
    delete ui;
}

void tiBackupAdd::on_comboBackupDevice_currentIndexChanged(int index)
{
    // Load available Backup partitions
    QString devname = ui->comboBackupDevice->itemData(index).toString();
    ui->comboBackupPartition->clear();

    qDebug() << "tiBackupAdd::on_comboBackupDevice_currentIndexChanged() -> selected dev2:" << devname;
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

void tiBackupAdd::on_comboBackupPartition_currentIndexChanged(int index)
{
    updatePartitionInformation();
}

void tiBackupAdd::on_btnSelectSource_clicked()
{
    QString startDir = (ui->leSourceFolder->text().isEmpty()) ? "/" : ui->leSourceFolder->text();

    QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Choose the source directory"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leSourceFolder->setText(dir);
}

void tiBackupAdd::on_btnSelectDest_clicked()
{
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QString uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    QString defaultPath = "/";

    qDebug() << "tiBackupAdd::on_btnSelectDest_clicked() -> selected part uuid:" << uuid;
    DeviceDisk selDisk;
    selDisk.devname = devname;

    DeviceDiskPartition part = selDisk.getPartitionByUUID(uuid);
    ui->lblDriveType->setText(part.type);

    TiBackupLib lib;
    if(lib.isMounted(part.name))
        defaultPath = lib.getMountDir(part.name);

    QString startDir = (ui->leDestFolder->text().isEmpty()) ? defaultPath : ui->leDestFolder->text();

    QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Choose the destination directory"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leDestFolder->setText(dir);

    qDebug() << "tiBackupAdd::on_btnSelectDest_clicked() -> generic name::" << TiBackupLib::convertPath2Generic(dir, lib.getMountDir(part.name));
}

void tiBackupAdd::on_btnAddBackupFolder_clicked()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());

    QStandardItem *item = new QStandardItem(ui->leSourceFolder->text());
    QStandardItem *item2 = new QStandardItem(ui->leDestFolder->text());

    int row = model->rowCount();
    model->setItem(row, 0, item);
    model->setItem(row, 1, item2);

    qDebug() << "tiBackupAdd::on_btnAddBackupFolder_clicked() -> rowcount::" << model->rowCount();
}

void tiBackupAdd::on_btnRemoveBackupFolder_clicked()
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

void tiBackupAdd::on_btnAddBackupJob_clicked()
{
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());

    if(ui->leBackupJobName->text().isEmpty())
    {
        QMessageBox::information(this, QString::fromUtf8("Add backupjob"), QString::fromUtf8("You must set a name for the backupjob."));
        return;
    }

    tiBackupJob job;
    job.name = ui->leBackupJobName->text();
    job.device = ui->comboBackupDevice->itemText(ui->comboBackupDevice->currentIndex());
    job.partition_uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    job.delete_add_file_on_dest = ui->cbDeleteAddFilesOnDest->isChecked();
    job.start_backup_on_hotplug = ui->cbBackupOnHotplug->isChecked();
    job.save_log = ui->cbSaveLog->isChecked();
    job.compare_via_checksum = ui->cbCompareViaChecksum->isChecked();
    job.notify = false;
    if(ui->gbNotify->isChecked() == true)
    {
        job.notify = true;
        job.notifyRecipients = ui->leNotifyRecipients->text();
    }
    job.scriptBeforeBackup = ui->leScriptPathBeforeBackup->text();
    job.scriptAfterBackup = ui->leScriptPathAfterBackup->text();

    DeviceDisk selDisk;
    selDisk.devname = devname;

    DeviceDiskPartition part = selDisk.getPartitionByUUID(job.partition_uuid);
    TiBackupLib lib;

    QHash<QString, QString> h;
    for(int i=0; i < model->rowCount(); i++)
    {
        h.insertMulti(model->item(i, 0)->text(), TiBackupLib::convertPath2Generic(model->item(i, 1)->text(), lib.getMountDir(part.name)));
    }
    job.backupdirs = h;

    // Set task values
    job.intervalType = static_cast<tiBackupJobInterval>(ui->cbInterval->currentIndex());
    job.intervalTime = "0";
    job.intervalDay = 0;
    switch(job.intervalType)
    {
    case tiBackupJobIntervalNONE:
        break;
    case tiBackupJobIntervalDAILY:
        job.intervalTime = ui->teDailyTime->text();
        break;
    case tiBackupJobIntervalWEEKLY:
        job.intervalTime = ui->teWeeklyTime->text();
        job.intervalDay = ui->cbWeeklyDay->currentIndex();
        break;
    case tiBackupJobIntervalMONTHLY:
        job.intervalTime = ui->teMonthlyTime->text();
        job.intervalDay = ui->sbMonthlyDay->value();
        break;
    }

    tiConfBackupJobs jobs;
    jobs.saveBackupJob(job);

    parentWidget()->close();

    emit jobAdded(job);
}

bool tiBackupAdd::eventFilter(QObject *object, QEvent *event)
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

void tiBackupAdd::on_btnPartitionMount_clicked()
{
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QString uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    qDebug() << "tiBackupAdd::on_btnPartitionMount_clicked() -> selected part uuid:" << uuid;
    DeviceDisk selDisk;
    selDisk.devname = devname;

    DeviceDiskPartition part = selDisk.getPartitionByUUID(uuid);

    TiBackupLib lib;
    if(lib.isMounted(part.name))
    {
        QMessageBox::information(this, QString::fromUtf8("Mountinformation"), QString::fromUtf8("The drive is already mounted."));
    }
    else
    {
        lib.mountPartition(&part);
        updatePartitionInformation();
    }
}

void tiBackupAdd::updatePartitionInformation()
{
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QString uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    qDebug() << "tiBackupAdd::updatePartitionInformation() -> selected part uuid:" << uuid;
    DeviceDisk selDisk;
    selDisk.devname = devname;

    DeviceDiskPartition part = selDisk.getPartitionByUUID(uuid);
    ui->lblDriveType->setText(part.type);

    TiBackupLib lib;
    if(lib.isMounted(part.name))
    {
        ui->btnPartitionMount->setDisabled(true);
        ui->lblMountInfo->setText(QString("Partition %1 is mounted on %2").arg(part.name, lib.getMountDir(part.name)));
    }
    else
    {
        ui->btnPartitionMount->setEnabled(true);
        ui->lblMountInfo->setText(QString("Partition %1 is not mounted").arg(part.name));
    }
}

void tiBackupAdd::on_btnCancel_clicked()
{
    parentWidget()->close();
}

void tiBackupAdd::on_btnEditScriptBeforeBackup_clicked()
{
    QMainWindow *winScriptEditor = new QMainWindow(this, Qt::Dialog);
    winScriptEditor->setWindowModality(Qt::WindowModal);
    winScriptEditor->setAttribute(Qt::WA_DeleteOnClose, true);
    QString path = ui->leScriptPathBeforeBackup->text();
    tiConfMain main_settings;

    if(path.isEmpty())
    {
        QDateTime currentDate = QDateTime::currentDateTime();
        path = QString("%1/%2_beforebackup.sh").arg(main_settings.getValue("paths/scripts").toString(), currentDate.toString("yyyyMMddhhmmss"));
        //ui->leScriptPathBeforeBackup->setText(path);
    }

    //tiPreferences *f = new tiPreferences(winScriptEditor);
    scriptEditor *e = new scriptEditor(winScriptEditor);
    e->loadScript(path);
    QObject::connect(e, SIGNAL(scriptSaved(QString)), this, SLOT(onScriptBeforeChanged(QString)));
    winScriptEditor->setCentralWidget(e);
    winScriptEditor->setMinimumSize(QSize(e->width(),e->height()));
    //winScriptEditor->setMaximumSize(QSize(e->width(),e->height()));
    winScriptEditor->setWindowTitle(windowTitle() + QObject::trUtf8(" - Script Editor"));

    winScriptEditor->show();
}

void tiBackupAdd::onScriptBeforeChanged(QString scriptPath)
{
    ui->leScriptPathBeforeBackup->setText(scriptPath);
}

void tiBackupAdd::on_btnEditScriptAfterBackup_clicked()
{
    QMainWindow *winScriptEditor = new QMainWindow(this, Qt::Dialog);
    winScriptEditor->setWindowModality(Qt::WindowModal);
    winScriptEditor->setAttribute(Qt::WA_DeleteOnClose, true);
    QString path = ui->leScriptPathAfterBackup->text();
    tiConfMain main_settings;

    if(path.isEmpty())
    {
        QDateTime currentDate = QDateTime::currentDateTime();
        path = QString("%1/%2_afterbackup.sh").arg(main_settings.getValue("paths/scripts").toString(), currentDate.toString("yyyyMMddhhmmss"));
        //ui->leScriptPathBeforeBackup->setText(path);
    }

    //tiPreferences *f = new tiPreferences(winScriptEditor);
    scriptEditor *e = new scriptEditor(winScriptEditor);
    e->loadScript(path);
    QObject::connect(e, SIGNAL(scriptSaved(QString)), this, SLOT(onScriptAfterChanged(QString)));
    winScriptEditor->setCentralWidget(e);
    winScriptEditor->setMinimumSize(QSize(e->width(),e->height()));
    //winScriptEditor->setMaximumSize(QSize(e->width(),e->height()));
    winScriptEditor->setWindowTitle(windowTitle() + QObject::trUtf8(" - Script Editor"));

    winScriptEditor->show();
}

void tiBackupAdd::onScriptAfterChanged(QString scriptPath)
{
    ui->leScriptPathAfterBackup->setText(scriptPath);
}
