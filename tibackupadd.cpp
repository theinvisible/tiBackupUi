#include "tibackupadd.h"
#include "ui_tibackupadd.h"

#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMessageBox>

#include "ticonf.h"
#include "tibackuplib.h"

tiBackupAdd::tiBackupAdd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tiBackupAdd)
{
    ui->setupUi(this);

    int i=0;

    // Load available Backup devices
    TiBackupLib blib;
    QList<DeviceDisk> disks = blib.getAttachedDisks();
    qDebug() << "disks found:" << disks.count();

    for(i=0; i < disks.count(); i++)
    {
        DeviceDisk disk = disks.at(i);

        qDebug() << "disk:" << disk.devname;
        if(disk.devtype == "disk")
        {
            ui->comboBackupDevice->insertItem(0, QString("%1 - %2 (%3)").arg(disk.vendor, disk.model, disk.devname), disk.devname);
        }
    }

    // Load available Backup partitions
    /*
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();

    qDebug() << "selected dev:" << devname;
    DeviceDisk selDisk;
    selDisk.devname = devname;
    selDisk.readPartitions();
    for(i=0; i < selDisk.partitions.count(); i++)
    {
        DeviceDiskPartition part = selDisk.partitions.at(i);
        ui->comboBackupPartition->insertItem(0, QString("%1 (%2)").arg(part.name, part.uuid));
    }
    */

    parent->installEventFilter(this);

    QStringList headers;
    headers << "Quellordner" << "Zielordner";

    QStandardItemModel *model = new QStandardItemModel(ui->tvBackupFolders);
    model->setHorizontalHeaderLabels(headers);

    ui->tvBackupFolders->setModel(model);
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

    qDebug() << "selected dev2:" << devname;
    DeviceDisk selDisk;
    selDisk.devname = devname;
    selDisk.readPartitions();
    for(int i=0; i < selDisk.partitions.count(); i++)
    {
        DeviceDiskPartition part = selDisk.partitions.at(i);
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

    QString dir = QFileDialog::getExistingDirectory(this, tr("Bitte wählen Sie das Quellverzeichnis"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leSourceFolder->setText(dir);
}

void tiBackupAdd::on_btnSelectDest_clicked()
{
    QString startDir = (ui->leDestFolder->text().isEmpty()) ? "/" : ui->leDestFolder->text();

    QString dir = QFileDialog::getExistingDirectory(this, tr("Bitte wählen Sie das Zielverzeichnis"),
                                                    startDir,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);

    ui->leDestFolder->setText(dir);
}

void tiBackupAdd::on_btnAddBackupFolder_clicked()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());

    QStandardItem *item = new QStandardItem(ui->leSourceFolder->text());
    QStandardItem *item2 = new QStandardItem(ui->leDestFolder->text());

    int row = model->rowCount();
    model->setItem(row, 0, item);
    model->setItem(row, 1, item2);

    qDebug() << "rowcount::" << model->rowCount();
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
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());

    QHash<QString, QString> h;
    //h.insertMulti("/tmp", "/disk2");
    //h.insertMulti("/tmp2", "/disk3");

    //model->
    for(int i=0; i < model->rowCount(); i++)
    {
        h.insertMulti(model->item(i, 0)->text(), model->item(i, 1)->text());
    }

    tiBackupJob job;
    job.name = ui->leBackupJobName->text();
    job.device = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    job.partition_uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    job.backupdirs = h;
    job.delete_add_file_on_dest = true;

    tiConfBackupJobs jobs;
    jobs.saveBackupJob(job);
}

bool tiBackupAdd::eventFilter(QObject *object, QEvent *event)
{
    if(object == parentWidget() && event->type() == QEvent::Close)
    {
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
    }

    return false;
}

void tiBackupAdd::on_btnPartitionMount_clicked()
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

void tiBackupAdd::updatePartitionInformation()
{
    QString devname = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    QString uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    qDebug() << "selected part uuid:" << uuid;
    DeviceDisk selDisk;
    selDisk.devname = devname;

    DeviceDiskPartition part = selDisk.getPartitionByUUID(uuid);
    ui->lblDriveType->setText(part.type);

    TiBackupLib lib;
    if(lib.isMounted(part.name))
    {
        ui->lblMountInfo->setText(QString("Partition %1 ist gemounted auf %2").arg(part.name, lib.getMountDir(part.name)));
    }
    else
    {
        ui->lblMountInfo->setText(QString("Partition %1 ist nicht gemounted").arg(part.name));
    }
}
