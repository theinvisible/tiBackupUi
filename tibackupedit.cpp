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
#include <QMainWindow>
#include <QDateTime>

#include "tools/scripteditor.h"

tiBackupEdit::tiBackupEdit(QWidget *parent, tiBackupJob *job) :
    QWidget(parent),
    ui(new Ui::tiBackupEdit)
{
    ui->setupUi(this);
    currentJob = job;
    currentJobDiskisAttached = false;
    ui->btnPartitionMount->setDisabled(true);

    parent->installEventFilter(this);

    QStringList headers;
    headers << "Quellordner" << "Destinationfolder";

    QStandardItemModel *model = new QStandardItemModel(ui->tvBackupFolders);
    model->setHorizontalHeaderLabels(headers);

    ui->tvBackupFolders->setModel(model);
    ui->tvBackupFolders->header()->resizeSection(0, 350);

    // Load available Backup devices
    TiBackupLib blib;
    QList<DeviceDisk> disks = blib.getAttachedDisks();
    qDebug() << "tiBackupEdit::tiBackupEdit() -> disks found:" << disks.count();

    for(int i=0; i < disks.count(); i++)
    {
        DeviceDisk disk = disks.at(i);

        qDebug() << "tiBackupEdit::tiBackupEdit() -> disk:" << disk.devname;
        if(disk.devtype == "disk")
        {
            ui->comboBackupDevice->insertItem(0, QString("%1 - %2 (%3)").arg(disk.vendor, disk.model, disk.devname), disk.devname);
        }
    }

    updateJobDetails();
    if(currentJobDiskisAttached == true)
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
    ui->cbCompareViaChecksum->setChecked(currentJob->compare_via_checksum);
    ui->leNotifyRecipients->setText(currentJob->notifyRecipients);
    if(currentJob->notify == true)
    {
        ui->gbNotify->setChecked(true);
    }
    ui->leScriptPathBeforeBackup->setText(currentJob->scriptBeforeBackup);
    ui->leScriptPathAfterBackup->setText(currentJob->scriptAfterBackup);

    // Set defined task if any
    switch(currentJob->intervalType)
    {
    case tiBackupJobIntervalNONE:
        break;
    case tiBackupJobIntervalDAILY:
    {
        ui->cbInterval->setCurrentIndex(static_cast<int>(currentJob->intervalType));
        ui->teDailyTime->setDateTime(QDateTime::fromString(currentJob->intervalTime, "hh:mm"));
        break;
    }
    case tiBackupJobIntervalWEEKLY:
        ui->cbInterval->setCurrentIndex(static_cast<int>(currentJob->intervalType));
        ui->teWeeklyTime->setDateTime(QDateTime::fromString(currentJob->intervalTime, "hh:mm"));
        ui->cbWeeklyDay->setCurrentIndex(currentJob->intervalDay);
        break;
    case tiBackupJobIntervalMONTHLY:
        ui->cbInterval->setCurrentIndex(static_cast<int>(currentJob->intervalType));
        ui->teMonthlyTime->setDateTime(QDateTime::fromString(currentJob->intervalTime, "hh:mm"));
        ui->sbMonthlyDay->setValue(currentJob->intervalDay);
        break;
    }

    // We must see if the current job disk is attached
    // Load available Backup devices
    TiBackupLib blib;
    QList<DeviceDisk> disks = blib.getAttachedDisks();
    for(int i=0; i < disks.count(); i++)
    {
        DeviceDisk disk = disks.at(i);

        qDebug() << "tiBackupEdit::updateJobDetails() -> disk:" << disk.devname;
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
                    qDebug() << "tiBackupEdit::updateJobDetails() -> job disk is attached right now";

                    int devrow = ui->comboBackupDevice->findData(disk.devname);
                    int partrow = ui->comboBackupPartition->findData(part.uuid);
                    qDebug() << "tiBackupEdit::updateJobDetails() -> devrow::" << devrow << "::partrow::" << partrow;

                    ui->comboBackupDevice->setCurrentIndex(devrow);
                    ui->comboBackupPartition->setCurrentIndex(partrow);

                    currentJobDiskisAttached = true;

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
        ui->lblMountInfo->setText(QString("Partition %1 is mounted on %2").arg(part.name, lib.getMountDir(part.name)));
    }
    else
    {
        ui->btnPartitionMount->setEnabled(true);
        ui->lblMountInfo->setText(QString("Partition %1 is not mounted").arg(part.name));
    }
}

QString tiBackupEdit::getBackupDeviceValue()
{
    QString selDevname = ui->comboBackupDevice->itemText(ui->comboBackupDevice->currentIndex());
    QString editDevname = ui->comboBackupDevice->currentText();

    qDebug() << "tiBackupEdit::getBackupDeviceValue() -> selDev::" << selDevname << "::editDev::" << editDevname << "::";

    if(currentJobDiskisAttached == true)
    {
        return selDevname;
    }
    else
    {
        if(selDevname == editDevname)
        {
            return selDevname;
        }
        else
        {
            return editDevname;
        }
    }
}

QString tiBackupEdit::getBackupPartitionValue()
{
    QString selPartition = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    QString editPartition = ui->comboBackupPartition->currentText();

    qDebug() << "tiBackupEdit::getBackupPartitionValue() -> selPart::" << selPartition << "::editPart::" << editPartition << "::";

    if(currentJobDiskisAttached == true)
    {
        return selPartition;
    }
    else
    {
        if(selPartition == editPartition)
        {
            return selPartition;
        }
        else
        {
            if(ui->comboBackupPartition->itemText(ui->comboBackupPartition->currentIndex()) == editPartition)
                return selPartition;

            return editPartition;
        }
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

    QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Choose the source directory"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leSourceFolder->setText(dir);
}

void tiBackupEdit::on_btnSelectDest_clicked()
{
    QString defaultPath = "/";

    TiBackupLib lib;
    DeviceDiskPartition part = TiBackupLib::getPartitionByUUID(getBackupPartitionValue());
    if(lib.isMounted(part.name))
        defaultPath = lib.getMountDir(part.name);

    QString startDir = (ui->leDestFolder->text().isEmpty()) ? defaultPath : ui->leDestFolder->text();

    QString dir = QFileDialog::getExistingDirectory(this, trUtf8("Choose the destination directory"),
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
    qDebug() << "tiBackupEdit::on_btnEditBackupJob_clicked() -> BackupDeviceValue::" << getBackupDeviceValue();
    qDebug() << "tiBackupEdit::on_btnEditBackupJob_clicked() -> BackupPartitionValue::" << getBackupPartitionValue();

    //QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());
    tiConfBackupJobs jobs;

    if(ui->leBackupJobName->text().isEmpty())
    {
        QMessageBox::information(this, QString::fromUtf8("Edit backupjob"), QString::fromUtf8("You must set a name for the backupjob."));
        return;
    }

    tiBackupJob job = *currentJob;

    if(currentJob->name != ui->leBackupJobName->text())
    {
        // We must rename the job
        if(!jobs.renameJob(currentJob->name, ui->leBackupJobName->text()))
        {
            QMessageBox::information(this, QString::fromUtf8("Edit backupjob"), QString::fromUtf8("The backupjob name could not be changed."));
            return;
        }
        job.name = ui->leBackupJobName->text();
    }

    job.device = getBackupDeviceValue();
    job.partition_uuid = getBackupPartitionValue();
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

    /*
    DeviceDisk selDisk;
    selDisk.devname = devname;

    DeviceDiskPartition part = selDisk.getPartitionByUUID(job.partition_uuid);
    TiBackupLib lib;
    */

    QHash<QString, QString> h;
    QString dest;
    TiBackupLib lib;
    bool diskMounted = false;
    DeviceDiskPartition part = TiBackupLib::getPartitionByUUID(getBackupPartitionValue());
    if(lib.isMounted(part.name))
        diskMounted = true;

    for(int i=0; i < model->rowCount(); i++)
    {
        dest = model->item(i, 1)->text();
        if(diskMounted == true)
            dest = TiBackupLib::convertPath2Generic(dest, lib.getMountDir(part.name));

        //h.insertMulti(model->item(i, 0)->text(), TiBackupLib::convertPath2Generic(model->item(i, 1)->text(), lib.getMountDir(part.name)));
        h.insertMulti(model->item(i, 0)->text(), dest);
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

    jobs.saveBackupJob(job);

    parentWidget()->close();

    emit jobEdited(job);
}

void tiBackupEdit::on_comboBackupDevice_currentIndexChanged(int index)
{
    // Load available Backup partitions
    QString devname = ui->comboBackupDevice->itemData(index).toString();
    ui->comboBackupPartition->clear();

    qDebug() << "tiBackupEdit::on_comboBackupDevice_currentIndexChanged() -> devname" << devname;
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
    ;
}

void tiBackupEdit::on_btnPartitionMount_clicked()
{
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QString uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    qDebug() << "tiBackupEdit::on_btnPartitionMount_clicked() -> selected part uuid:" << uuid;
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

void tiBackupEdit::on_comboBackupPartition_activated(int index)
{
    updatePartitionInformation();
}

void tiBackupEdit::on_btnEditScriptBeforeBackup_clicked()
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
    QObject::connect(e, SIGNAL(scriptSaved(QString)), this, SLOT(onScriptBeforeCchanged(QString)));
    winScriptEditor->setCentralWidget(e);
    winScriptEditor->setMinimumSize(QSize(e->width(),e->height()));
    //winScriptEditor->setMaximumSize(QSize(e->width(),e->height()));
    winScriptEditor->setWindowTitle(windowTitle() + QObject::trUtf8(" - Script Editor"));

    winScriptEditor->show();

    qDebug() << "tiBackupAdd::on_btnEditScriptBeforeBackup_clicked() -> test test";
}

void tiBackupEdit::onScriptBeforeCchanged(QString scriptPath)
{
    ui->leScriptPathBeforeBackup->setText(scriptPath);
}

void tiBackupEdit::on_btnEditScriptAfterBackup_clicked()
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
        //ui->leScriptPathAfterBackup->setText(path);
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

    qDebug() << "tiBackupAdd::on_btnEditScriptAfterBackup_clicked() -> test test";
}

void tiBackupEdit::onScriptAfterChanged(QString scriptPath)
{
    ui->leScriptPathAfterBackup->setText(scriptPath);
}
