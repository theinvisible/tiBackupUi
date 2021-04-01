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
#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonArray>

#include "config.h"
#include "tibackupapi.h"
#include "ipcclient.h"
#include "pbsclient.h"

#include "tools/scripteditor.h"
#include "tools/pbsmanager.h"

tiBackupAdd::tiBackupAdd(tiBackupAddMode mode, tiBackupJob *job, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tiBackupAdd)
{
    ui->setupUi(this);
    formmode = mode;

    ui->btnPartitionMount->setDisabled(true);

    QStringList headers;
    headers << tr("Name") << tr("Backup ID") << tr("Backup Type") << tr("Last backup") << tr("Backup count");
    QStandardItemModel *model = new QStandardItemModel(ui->tvPBServer);
    model->setHorizontalHeaderLabels(headers);
    ui->tvPBServer->setModel(model);
    ui->tvPBServer->sortByColumn(0, Qt::AscendingOrder);

    ui->tvPBServer->header()->resizeSection(0, 250);
    ui->tvPBServer->header()->resizeSection(1, 80);
    updatePBServers();

    if(formmode == tiBackupAddModeAdd)
    {
        refreshBackupDevices();

        parent->installEventFilter(this);

        QStringList headers;
        headers << "Source" << "Destination";

        QStandardItemModel *model = new QStandardItemModel(ui->tvBackupFolders);
        model->setHorizontalHeaderLabels(headers);

        ui->tvBackupFolders->setModel(model);
        ui->tvBackupFolders->header()->resizeSection(0, 350);
    }
    else if(formmode == tiBackupAddModeEdit)
    {
        currentJob = job;
        currentJobDiskisAttached = false;
        ui->btnPartitionMount->setDisabled(true);

        parent->installEventFilter(this);

        QStringList headers;
        headers << "Source" << "Destination";

        QStandardItemModel *model = new QStandardItemModel(ui->tvBackupFolders);
        model->setHorizontalHeaderLabels(headers);

        ui->tvBackupFolders->setModel(model);
        ui->tvBackupFolders->header()->resizeSection(0, 350);

        refreshBackupDevices();

        updateJobDetails();
        if(currentJobDiskisAttached == true)
            updatePartitionInformation();
    }
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

    ipcClient *client = ipcClient::instance();
    QList<DeviceDiskPartition> partitions = client->getPartitionsForDevName(devname);

    for(int i=0; i < partitions.count(); i++)
    {
        DeviceDiskPartition part = partitions.at(i);

        if(part.uuid.isEmpty())
            continue;

        ui->comboBackupPartition->insertItem(0, QString("%1 (%2)").arg(part.name, part.uuid), part.uuid);
    }
}

void tiBackupAdd::on_comboBackupPartition_currentIndexChanged(__attribute__ ((unused)) int index)
{
    updatePartitionInformation();
}

void tiBackupAdd::on_btnSelectSource_clicked()
{
    QString startDir = (ui->leSourceFolder->text().isEmpty()) ? "/" : ui->leSourceFolder->text();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the source directory"),
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
    //DeviceDisk selDisk;
    //selDisk.devname = devname;

    //DeviceDiskPartition part = selDisk.getPartitionByUUID(uuid);
    ipcClient *client = ipcClient::instance();
    DeviceDiskPartition part = client->getPartitionByDevnameUUID(devname, uuid);
    ui->lblDriveType->setText(part.type);

    TiBackupLib lib;
    if(lib.isMounted(&part))
        defaultPath = lib.getMountDir(&part);

    QString startDir = (ui->leDestFolder->text().isEmpty()) ? defaultPath : ui->leDestFolder->text();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the destination directory"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leDestFolder->setText(dir);

    qDebug() << "tiBackupAdd::on_btnSelectDest_clicked() -> generic name::" << TiBackupLib::convertPath2Generic(dir, lib.getMountDir(&part));
}

void tiBackupAdd::on_btnAddBackupFolder_clicked()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());
    QString src = ui->leSourceFolder->text();
    QString dst = ui->leDestFolder->text();

    if(!QFile::exists(src) || !QFile::exists(dst))
    {
        QMessageBox::warning(this, tr("Backup folders not found"),
                                    tr("Backup source or destination folder does not exist, please check your paths."));

        return;
    }

    QStandardItem *item = new QStandardItem(src);
    QStandardItem *item2 = new QStandardItem(dst);

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
    if(formmode == tiBackupAddModeAdd)
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

        /*
        DeviceDisk selDisk;
        selDisk.devname = devname;
        DeviceDiskPartition part = selDisk.getPartitionByUUID(job.partition_uuid);
        */
        ipcClient *client = ipcClient::instance();
        DeviceDiskPartition part = client->getPartitionByDevnameUUID(devname, job.partition_uuid);

        TiBackupLib lib;

        QHash<QString, QString> h;
        for(int i=0; i < model->rowCount(); i++)
        {
            h.insertMulti(model->item(i, 0)->text(), TiBackupLib::convertPath2Generic(model->item(i, 1)->text(), lib.getMountDir(&part)));
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

        // Set LUKS Enc values
        job.encLUKSType = static_cast<tiBackupEncLUKS>(ui->cbLUKSOptions->currentIndex());
        job.encLUKSFilePath = "";
        switch(job.encLUKSType)
        {
        case tiBackupEncLUKSNONE:
            break;
        case tiBackupEncLUKSFILE:
            job.encLUKSFilePath = ui->leLUKSFilePath->text();
            break;
        case tiBackupEncLUKSGENUSBDEV:
            break;
        }

        tiConfBackupJobs jobs;
        jobs.saveBackupJob(job);

        parentWidget()->close();

        emit jobAdded(job);
    }
    else if(formmode == tiBackupAddModeEdit)
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
        //DeviceDiskPartition part = TiBackupLib::getPartitionByUUID(getBackupPartitionValue());
        ipcClient *client = ipcClient::instance();
        DeviceDiskPartition part = client->getPartitionByUUID(getBackupPartitionValue());
        if(lib.isMounted(&part))
            diskMounted = true;

        for(int i=0; i < model->rowCount(); i++)
        {
            dest = model->item(i, 1)->text();
            if(diskMounted == true)
                dest = TiBackupLib::convertPath2Generic(dest, lib.getMountDir(&part));

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

        // Set LUKS Enc values
        job.encLUKSType = static_cast<tiBackupEncLUKS>(ui->cbLUKSOptions->currentIndex());
        job.encLUKSFilePath = "";
        switch(job.encLUKSType)
        {
        case tiBackupEncLUKSNONE:
            break;
        case tiBackupEncLUKSFILE:
            job.encLUKSFilePath = ui->leLUKSFilePath->text();
            break;
        case tiBackupEncLUKSGENUSBDEV:
            break;
        }

        jobs.saveBackupJob(job);

        parentWidget()->close();

        emit jobEdited(job);
    }
}

QString tiBackupAdd::getBackupDeviceValue()
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

QString tiBackupAdd::getBackupPartitionValue()
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

bool tiBackupAdd::eventFilter(QObject *object, QEvent *event)
{
    if(object == parentWidget() && event->type() == QEvent::Close)
    {
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
    if(lib.isMounted(&part))
    {
        QMessageBox::information(this, QString::fromUtf8("Mountinformation"), QString::fromUtf8("The drive is already mounted."));
    }
    else
    {
        tiBackupJob job;
        job.encLUKSType = static_cast<tiBackupEncLUKS>(ui->cbLUKSOptions->currentIndex());
        job.encLUKSFilePath = ui->leLUKSFilePath->text();

        lib.mountPartition(&part, &job);
        updatePartitionInformation();
    }
}

void tiBackupAdd::updatePartitionInformation()
{
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QString uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    qDebug() << "tiBackupAdd::updatePartitionInformation() -> selected part uuid:" << uuid;

    ipcClient *client = ipcClient::instance();
    DeviceDiskPartition part = client->getPartitionByDevnameUUID(devname, uuid);

    ui->lblDriveType->setText(part.type);

    TiBackupLib lib;
    if(lib.isMounted(&part))
    {
        ui->btnPartitionMount->setDisabled(true);
        ui->lblMountInfo->setText(QString("Partition %1 is mounted on %2").arg(part.name, lib.getMountDir(&part)));
    }
    else
    {
        ui->btnPartitionMount->setEnabled(true);
        ui->lblMountInfo->setText(QString("Partition %1 is not mounted").arg(part.name));
    }
}

void tiBackupAdd::refreshBackupDevices()
{
    ui->comboBackupDevice->clear();

    // Load available Backup devices
    TiBackupLib blib;
    QList<DeviceDisk> disks = blib.getAttachedDisks();
    qDebug() << "tiBackupAdd::tiBackupAdd() -> disks found:" << disks.count();

    for(int i=0; i < disks.count(); i++)
    {
        DeviceDisk disk = disks.at(i);

        qDebug() << "tiBackupAdd::tiBackupAdd() -> disk:" << disk.devname;
        if(disk.devtype == "disk")
        {
            ui->comboBackupDevice->insertItem(0, QString("%1 - %2 (%3)").arg(disk.vendor, disk.model, disk.devname), disk.devname);
        }
    }
}

void tiBackupAdd::on_btnCancel_clicked()
{
    parentWidget()->close();
}

void tiBackupAdd::on_btnRefreshDevices_clicked()
{
    refreshBackupDevices();
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
    winScriptEditor->setWindowTitle(windowTitle() + QObject::tr(" - Script Editor"));

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
    winScriptEditor->setWindowTitle(windowTitle() + QObject::tr(" - Script Editor"));

    winScriptEditor->show();
}

void tiBackupAdd::onScriptAfterChanged(QString scriptPath)
{
    ui->leScriptPathAfterBackup->setText(scriptPath);
}

void tiBackupAdd::on_btnLUKSFileSelector_clicked()
{
    QString startDir = (ui->leLUKSFilePath->text().isEmpty()) ? "/" : ui->leLUKSFilePath->text();

    QString file = QFileDialog::getOpenFileName(this, tr("Choose the password file"), startDir);

    ui->leLUKSFilePath->setText(file);
}

void tiBackupAdd::updateJobDetails()
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
        ui->swIntervalSettings->setCurrentIndex(static_cast<int>(currentJob->intervalType));
        ui->teDailyTime->setDateTime(QDateTime::fromString(currentJob->intervalTime, "hh:mm"));
        break;
    }
    case tiBackupJobIntervalWEEKLY:
        ui->cbInterval->setCurrentIndex(static_cast<int>(currentJob->intervalType));
        ui->swIntervalSettings->setCurrentIndex(static_cast<int>(currentJob->intervalType));
        ui->teWeeklyTime->setDateTime(QDateTime::fromString(currentJob->intervalTime, "hh:mm"));
        ui->cbWeeklyDay->setCurrentIndex(currentJob->intervalDay);
        break;
    case tiBackupJobIntervalMONTHLY:
        ui->cbInterval->setCurrentIndex(static_cast<int>(currentJob->intervalType));
        ui->swIntervalSettings->setCurrentIndex(static_cast<int>(currentJob->intervalType));
        ui->teMonthlyTime->setDateTime(QDateTime::fromString(currentJob->intervalTime, "hh:mm"));
        ui->sbMonthlyDay->setValue(currentJob->intervalDay);
        break;
    }

    // Set defined LUKS settings if any
    switch(currentJob->encLUKSType)
    {
    case tiBackupEncLUKSNONE:
        break;
    case tiBackupEncLUKSFILE:
    {
        ui->cbLUKSOptions->setCurrentIndex(static_cast<int>(currentJob->encLUKSType));
        ui->swLUKSOptions->setCurrentIndex(static_cast<int>(currentJob->encLUKSType));
        ui->leLUKSFilePath->setText(currentJob->encLUKSFilePath);
        break;
    }
    case tiBackupEncLUKSGENUSBDEV:
        ui->cbLUKSOptions->setCurrentIndex(static_cast<int>(currentJob->encLUKSType));
        ui->swLUKSOptions->setCurrentIndex(static_cast<int>(currentJob->encLUKSType));
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
            ipcClient *client = ipcClient::instance();
            QList<DeviceDiskPartition> partitions = client->getPartitionsForDevName(disk.devname);

            for(int j=0; j < partitions.count(); j++)
            {
                DeviceDiskPartition part = partitions.at(j);

                if(part.uuid.isEmpty())
                    continue;

                if(part.uuid == currentJob->partition_uuid)
                {
                    // Job disk is attached right now
                    qDebug() << "tiBackupEdit::updateJobDetails() -> job disk is attached right now";

                    int devrow = ui->comboBackupDevice->findData(disk.devname);
                    ui->comboBackupDevice->setCurrentIndex(devrow);

                    int partrow = ui->comboBackupPartition->findData(part.uuid);
                    ui->comboBackupPartition->setCurrentIndex(partrow);
                    qDebug() << "tiBackupEdit::updateJobDetails() -> devrow::" << devrow << "::partrow::" << partrow;

                    currentJobDiskisAttached = true;

                    return;
                }
            }
        }
    }
}

void tiBackupAdd::on_btnPBSManage_clicked()
{
    PBSManager *d = new PBSManager(this);
    connect(d, &PBSManager::form_finished, this, [=]() { updatePBServers(); });
    d->show();
}

void tiBackupAdd::updatePBServers()
{
    tiConfPBServers *ticonfpbs = tiConfPBServers::instance();
    ticonfpbs->readItems();
    ui->comboPBServer->clear();
    QList<PBServer*> pbs = ticonfpbs->getItems();
    for(int i=0; i < pbs.count(); i++)
    {
        PBServer *pb = pbs.at(i);
        ui->comboPBServer->insertItem(0, QString("%1 - %2").arg(pb->name, pb->host), pb->uuid);
    }
}

void tiBackupAdd::on_btnPBSConnect_clicked()
{
    QString selPBServer = ui->comboPBServer->itemData(ui->comboPBServer->currentIndex()).toString();
    tiConfPBServers *ticonfpbs = tiConfPBServers::instance();
    ticonfpbs->readItems();
    PBServer *pb = ticonfpbs->getItemByUuid(selPBServer);
    ui->comboPBSDatastore->clear();

    pbsClient *pbs = pbsClient::instance();
    HttpStatus::Code status = pbs->auth(pb->host, pb->port, pb->username, pb->password);
    if(status == HttpStatus::Code::OK)
    {
        pbsClient::HttpResponse resp = pbs->getDatastores();
        if(resp.status == HttpStatus::Code::OK)
        {
            QJsonArray datastores = resp.data.object()["data"].toArray();
            for(int i=0; i < datastores.size(); i++)
            {
                QJsonObject datastore = datastores[i].toObject();
                ui->comboPBSDatastore->insertItem(0, QString("%1 - %2").arg(datastore["store"].toString(), datastore["comment"].toString()), datastore["store"].toString());
            }
        }
    }
}

void tiBackupAdd::on_comboPBSDatastore_currentIndexChanged(int index)
{
    QString selPBServer = ui->comboPBServer->itemData(ui->comboPBServer->currentIndex()).toString();
    QString selPBSDatastore = ui->comboPBSDatastore->itemData(index).toString();
    tiConfPBServers *ticonfpbs = tiConfPBServers::instance();
    ticonfpbs->readItems();
    PBServer *pb = ticonfpbs->getItemByUuid(selPBServer);

    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvPBServer->model());
    model->removeRows(0, model->rowCount());
    int row = model->rowCount();

    QStandardItem *item = 0;
    QStandardItem *item2 = 0;
    QStandardItem *item3 = 0;
    QStandardItem *item4 = 0;
    QStandardItem *item5 = 0;

    pbsClient *pbs = pbsClient::instance();
    HttpStatus::Code status = pbs->auth(pb->host, pb->port, pb->username, pb->password);
    if(status == HttpStatus::Code::OK)
    {
        pbsClient::HttpResponse resp = pbs->getDatastoreGroups(selPBSDatastore);
        if(resp.status == HttpStatus::Code::OK)
        {
            QJsonArray groups = resp.data.object()["data"].toArray();
            for(int i=0; i < groups.size(); i++)
            {
                QJsonObject group = groups[i].toObject();
                qInfo() << "group" << group;

                QString file = "";
                if(group["backup-type"].toString() == "vm")
                    file = "qemu-server.conf.blob";
                else if(group["backup-type"].toString() == "ct")
                    file = "pct.conf.blob";

                pbsClient::HttpResponseRaw resp2 = pbs->getBackupFile(selPBSDatastore, group["backup-id"].toString(), group["last-backup"].toInt(), group["backup-type"].toString(), file);
                qInfo() << "qemuserverdata::" << resp2.data;

                item = new QStandardItem("Name");
                item->setCheckable(true);
                item2 = new QStandardItem(group["backup-id"].toString());
                item3 = new QStandardItem(group["backup-type"].toString());
                item4 = new QStandardItem(QString::number(group["last-backup"].toInt()));
                item5 = new QStandardItem(QString::number(group["backup-count"].toInt()));

                row = model->rowCount();
                model->setItem(row, 0, item);
                model->setItem(row, 1, item2);
                model->setItem(row, 2, item3);
                model->setItem(row, 3, item4);
                model->setItem(row, 4, item5);
            }
        }
    }

    ui->tvPBServer->setSortingEnabled(true);
    ui->tvPBServer->sortByColumn(1, Qt::AscendingOrder);
}
