#include "tibackupadd.h"
#include "ui_tibackupadd.h"

#include <QDebug>
#include <QFileDialog>
#include <QStandardItemModel>

#include "ticonf.h"

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

    qDebug() << "selected dev:" << devname;
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
    QString devname = ui->comboBackupDevice->itemData(index).toString();
    QString uuid = ui->comboBackupPartition->itemData(index).toString();
    qDebug() << "selected part uuid:" << uuid;
    DeviceDisk selDisk;
    selDisk.devname = devname;
    DeviceDiskPartition part = selDisk.getPartitionByUUID(uuid);
    ui->lblDriveType->setText(part.type);
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
}

void tiBackupAdd::on_btnRemoveBackupFolder_clicked()
{
    //QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvBackupFolders->model());
    //ui->tvBackupFolders->selectedItems();
}

void tiBackupAdd::on_btnAddBackupJob_clicked()
{
    QHash<QString, QString> h;
    h.insertMulti("/tmp", "/disk2");
    h.insertMulti("/tmp2", "/disk3");

    tiBackupJob job;
    job.name = ui->leBackupJobName->text();
    job.device = ui->comboBackupDevice->itemData(ui->comboBackupDevice->currentIndex()).toString();
    job.partition_uuid = ui->comboBackupPartition->itemData(ui->comboBackupPartition->currentIndex()).toString();
    job.backupdirs = h;
    job.delete_add_file_on_dest = true;

    tiConfBackupJobs jobs;
    jobs.saveBackupJob(job);
}
