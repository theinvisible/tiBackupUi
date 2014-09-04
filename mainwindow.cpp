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
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(onActionPreferences()));

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

void MainWindow::onjobEdited(tiBackupJob job)
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

    ui->tvAvailBackupJobs->header()->resizeSection(0, 150);
    ui->tvAvailBackupJobs->header()->resizeSection(1, 300);
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

    tiBackupEdit *f = new tiBackupEdit(prefWindow, jobs.getJobByName(jobName));
    prefWindow->setCentralWidget(f);
    prefWindow->setMinimumSize(QSize(f->width(),f->height()));
    prefWindow->setMaximumSize(QSize(f->width(),f->height()));
    prefWindow->setWindowTitle(windowTitle() + QObject::trUtf8(" - Backupjob <%1> bearbeiten").arg(jobName));

    connect(f, SIGNAL(jobEdited(tiBackupJob)), this, SLOT(onjobEdited(tiBackupJob)));
    prefWindow->show();
}

void MainWindow::on_tvAvailBackupJobs_doubleClicked(const QModelIndex &index)
{
    on_btnBackupJobEdit_clicked();
}

void MainWindow::onActionPreferences()
{
    QMainWindow *prefWindow = new QMainWindow(this, Qt::Dialog);
    prefWindow->setWindowModality(Qt::WindowModal);

    tiPreferences *f = new tiPreferences(prefWindow);
    prefWindow->setCentralWidget(f);
    prefWindow->setMinimumSize(QSize(f->width(),f->height()));
    prefWindow->setMaximumSize(QSize(f->width(),f->height()));
    prefWindow->setWindowTitle(windowTitle() + QObject::trUtf8(" - Einstellungen"));

    prefWindow->show();
}

void MainWindow::on_btnStartManualBackup_clicked()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvAvailBackupJobs->model());
    QItemSelectionModel *selmodel = ui->tvAvailBackupJobs->selectionModel();
    QModelIndexList sellist = selmodel->selectedRows();

    if(sellist.count() < 1)
    {
        return;
    }

    QString jobName = model->itemFromIndex(sellist.at(0))->text();
    tiConfBackupJobs jobss;

    tiBackupJob *job = jobss.getJobByName(jobName);

    // TODO Start this in own thread or GUI is blocked during backup
    job->startBackup();
}
