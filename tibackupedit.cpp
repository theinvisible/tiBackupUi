#include "tibackupedit.h"
#include "ui_tibackupedit.h"

#include <QStandardItemModel>
#include <QFileDialog>

tiBackupEdit::tiBackupEdit(QWidget *parent, tiBackupJob *job) :
    QWidget(parent),
    ui(new Ui::tiBackupEdit)
{
    ui->setupUi(this);
    currentJob = job;

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
