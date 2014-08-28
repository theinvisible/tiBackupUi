#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QDebug>
#include <QStandardItemModel>
#include <QMessageBox>

#include "ticonf.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Center window on startup
    QRect geom = QApplication::desktop()->availableGeometry();
    move((geom.width() - width()) / 2, (geom.height() - height()) / 2);

    QStringList headers;
    headers << "Name" << "Backuppartition UUID" << "Hotplug Backup";

    QStandardItemModel *model = new QStandardItemModel(ui->tvAvailBackupJobs);
    model->setHorizontalHeaderLabels(headers);

    ui->tvAvailBackupJobs->setModel(model);

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onActionAbout()));
    connect(ui->actionAddBackupjob, SIGNAL(triggered()), this, SLOT(on_btnAddBackup_clicked()));

    refreshBackupJobList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnAddBackup_clicked()
{
    QMainWindow *prefWindow = new QMainWindow(this, Qt::Dialog);
    prefWindow->setWindowModality(Qt::WindowModal);

    tiBackupAdd *f = new tiBackupAdd(prefWindow);
    prefWindow->setCentralWidget(f);
    prefWindow->setMinimumSize(QSize(f->width(),f->height()));
    prefWindow->setMaximumSize(QSize(f->width(),f->height()));
    prefWindow->setWindowTitle(windowTitle() + QObject::trUtf8(" - Backupjob hinzufügen"));

    connect(f, SIGNAL(jobAdded(tiBackupJob)), this, SLOT(onjobAdded(tiBackupJob)));
    prefWindow->show();
}

void MainWindow::onjobAdded(tiBackupJob job)
{
    refreshBackupJobList();
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this, trUtf8("Über tiBackup"), trUtf8("Entwickelt von: <p> <b>Rene Hadler</b> <br>"
                                                             "eMail: <a href=mailto:'rene@hadler.me'>rene@hadler.me</a> <br>"
                                                             "Website: <a href=https://hadler.me>https://hadler.me</a></p>"
                                                             "<p>libs...</p>"));
}

void MainWindow::refreshBackupJobList()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvAvailBackupJobs->model());
    tiConfBackupJobs jobss;
    jobss.readBackupJobs();

    model->removeRows(0, model->rowCount());

    QStandardItem *item = 0;
    QStandardItem *item2 = 0;
    QStandardItem *item3 = 0;
    int row = model->rowCount();

    QList<tiBackupJob*> jobs = jobss.getJobs();
    for(int i=0; i < jobs.count(); i++)
    {
        tiBackupJob *job = jobs.at(i);
        qDebug() << "jobs found::" << job->name;

        item = new QStandardItem(job->name);
        item2 = new QStandardItem(job->partition_uuid);
        item3 = new QStandardItem((job->start_backup_on_hotplug == true) ? "ja" : "nein");

        row = model->rowCount();
        model->setItem(row, 0, item);
        model->setItem(row, 1, item2);
        model->setItem(row, 2, item3);
    }
}

void MainWindow::on_btnBackupJobDelete_clicked()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvAvailBackupJobs->model());
    QItemSelectionModel *selmodel = ui->tvAvailBackupJobs->selectionModel();
    QModelIndexList sellist = selmodel->selectedRows();

    if(sellist.count() < 1)
    {
        return;
    }

    QString jobName = model->itemFromIndex(sellist.at(0))->text();
    //ui->tvBackupFolders->selectedItems();

    qDebug() << "remove backupjob with name::" << jobName;

    int ret = QMessageBox::warning(this, QString::fromUtf8("Backupjob löschen"),
                                QString::fromUtf8("Achtung, der gewählte Job wird unwiderruflich gelöscht, fortfahren?"),
                                QMessageBox::Yes | QMessageBox::No);

    switch(ret)
    {
    case QMessageBox::Yes:
        break;
    case QMessageBox::No:
    default:
        return;
    }

    tiConfBackupJobs jobss;
    if(jobss.removeJobByName(jobName))
    {
        refreshBackupJobList();
    }
    else
    {
        QMessageBox::information(this, trUtf8("Backupjob löschen"), trUtf8("Der Job konnte nicht gelöscht werden, ein Fehler ist aufgetreten."));
    }

}

void MainWindow::on_btnBackupJobEdit_clicked()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvAvailBackupJobs->model());
    QItemSelectionModel *selmodel = ui->tvAvailBackupJobs->selectionModel();
    QModelIndexList sellist = selmodel->selectedRows();

    if(sellist.count() < 1)
    {
        return;
    }

    QMainWindow *prefWindow = new QMainWindow(this, Qt::Dialog);
    prefWindow->setWindowModality(Qt::WindowModal);
    QString jobName = model->itemFromIndex(sellist.at(0))->text();

    tiConfBackupJobs jobs;

    tiBackupEdit *f = new tiBackupEdit(prefWindow, jobs.gebJobByName(jobName));
    prefWindow->setCentralWidget(f);
    prefWindow->setMinimumSize(QSize(f->width(),f->height()));
    prefWindow->setMaximumSize(QSize(f->width(),f->height()));
    prefWindow->setWindowTitle(windowTitle() + QObject::trUtf8(" - Backupjob bearbeiten"));

    //connect(f, SIGNAL(jobEdited(tiBackupJob)), this, SLOT(onjobEdited(tiBackupJob)));
    prefWindow->show();
}
