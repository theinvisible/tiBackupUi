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
#include <QFileSystemWatcher>
#include <QScrollBar>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QLocalSocket>
#include <QScreen>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "config.h"
#include "ticonf.h"
#include "tibackupapi.h"
#include "workers/tibackupjobworker.h"
#include "ipcclient.h"
#include "pbsclient.h"
#include "tools/pbsmanager.h"

#include <unistd.h>
#include <sys/types.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    main_settings = new tiConfMain;

    // Center window on startup
    QRect geom = QGuiApplication::screens()[0]->availableGeometry();
    if(geom.width() > 2560 && geom.height() > 1440)
        resize(geom.width() / 3, geom.height() / 3);
    move((geom.width() - width()) / 2, (geom.height() - height()) / 2);

    QStringList headers;
    headers << tr("Name") << tr("Devicename") << tr("Backuppartition UUID");

    QStandardItemModel *model = new QStandardItemModel(ui->tvAvailBackupJobs);
    model->setHorizontalHeaderLabels(headers);

    ui->tvAvailBackupJobs->setModel(model);

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onActionAbout()));
    connect(ui->actionAddBackupjob, SIGNAL(triggered()), this, SLOT(on_btnAddBackup_clicked()));
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(onActionPreferences()));
    connect(ui->actionPBSManager, &QAction::triggered, this, &MainWindow::onActionPBSManager);

    tiConfMain main_settings;
    QFileSystemWatcher *fileWatcher = new QFileSystemWatcher(this);
    fileWatcher->addPath(QString("%1/tibackup.log").arg(main_settings.getValue("paths/logs").toString()));
    connect(fileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(ontiBackupLogChanged(QString)));
    ontiBackupLogChanged("");

    lblTime = new QLabel(this);
    ui->statusBar->addPermanentWidget(lblTime);
    onTimeUpdate();

    // We start a timer for our clock
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeUpdate()));
    timer->start(1000);

    // Init service class
    service = new tiBackupService(this);
    connect(service, SIGNAL(serviceStarted()), this, SLOT(updateServiceStatus()));
    connect(service, SIGNAL(serviceStopped()), this, SLOT(updateServiceStatus()));
    connect(service, SIGNAL(serviceInstalled()), this, SLOT(updateServiceStatus()));

    // Check tiBackup IPC health
    ipcClient *client = ipcClient::instance();
    ipcClient::STATUS_ANSWER ret = client->checkHealth();
    if(ret.status != ipcClient::STATUS_OK)
    {
        QMessageBox::warning(this, tr("tiBackup Service Health Check"), tr("tiBackup Service is not running or answering! Please check status!"));
    }

    refreshBackupJobList();
    updateServiceStatus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnAddBackup_clicked()
{
    QMainWindow *prefWindow = new QMainWindow(this, Qt::Dialog);
    prefWindow->setWindowModality(Qt::WindowModal);

    tiBackupAdd *f = new tiBackupAdd(tiBackupAddModeAdd, 0, prefWindow);
    prefWindow->setCentralWidget(f);
    prefWindow->setMinimumSize(QSize(f->width(),f->height()));
    prefWindow->setMaximumSize(QSize(f->width(),f->height()));
    prefWindow->setWindowTitle(windowTitle() + QObject::tr(" - Add backupjob"));

    connect(f, SIGNAL(jobAdded(tiBackupJob)), this, SLOT(onjobAdded(tiBackupJob)));
    prefWindow->show();
}

void MainWindow::onjobAdded(__attribute__ ((unused)) tiBackupJob job)
{
    refreshBackupJobList();
}

void MainWindow::onjobEdited(__attribute__ ((unused)) tiBackupJob job)
{
    refreshBackupJobList();
}

void MainWindow::onActionAbout()
{
    QMessageBox::about(this, tr("About tiBackup"), tr("<b>tiBackup %1</b> <br><br>"
                                                             "Developer: <b>Rene Hadler</b> <br>"
                                                             "eMail: <a href=mailto:'rene@hadler.me'>rene@hadler.me</a> <br>"
                                                             "Website: <a href=https://hadler.me>https://hadler.me</a></p>"
                                                             "<p>libs...</p>").arg(tibackup_config::version));
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
        qDebug() << "MainWindow::refreshBackupJobList() -> jobs found::" << job->name;

        item = new QStandardItem(job->name);
        item2 = new QStandardItem(job->device);
        item3 = new QStandardItem(job->partition_uuid);

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

    qDebug() << "MainWindow::on_btnBackupJobDelete_clicked() -> remove backupjob with name::" << jobName;

    int ret = QMessageBox::warning(this, tr("Delete backupjob"),
                                tr("Warning, the selected backupjob will be deleted, continue?"),
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
        QMessageBox::information(this, tr("Delete backupjob"), tr("The selected backupjob could not be deleted, an error occured."));
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

    tiBackupAdd *f = new tiBackupAdd(tiBackupAddModeEdit, jobs.getJobByName(jobName), prefWindow);
    prefWindow->setCentralWidget(f);
    prefWindow->setMinimumSize(QSize(f->width(),f->height()));
    prefWindow->setMaximumSize(QSize(f->width(),f->height()));
    prefWindow->setWindowTitle(windowTitle() + QObject::tr(" - Edit backupjob <%1>").arg(jobName));

    connect(f, SIGNAL(jobEdited(tiBackupJob)), this, SLOT(onjobEdited(tiBackupJob)));
    prefWindow->show();
}

void MainWindow::on_tvAvailBackupJobs_doubleClicked(__attribute__ ((unused)) const QModelIndex &index)
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
    prefWindow->setWindowTitle(windowTitle() + QObject::tr(" - Preferences"));

    prefWindow->show();
}

void MainWindow::onActionPBSManager()
{
    PBSManager *d = new PBSManager(this);
    d->show();
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
    //ui->btnStartManualBackup->setDisabled(true);
    //ui->statusBar->showMessage(tr("Backupjob <%1> was started...").arg(jobName));

    ipcClient *client = ipcClient::instance();
    client->startBackup(jobName);

    /*
    QLocalSocket *apiClient = new QLocalSocket(this);
    connect(apiClient, SIGNAL(disconnected()), this, SLOT(onManualBackupFinished()));
    apiClient->connectToServer(tibackup_config::api_sock_name);
    if(apiClient->waitForConnected(1000))
    {
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(tibackup_config::ipc_version);
        QHash<tiBackupApi::API_VAR, QString> apiData;
        apiData[tiBackupApi::API_VAR::API_VAR_CMD] = tiBackupApi::API_CMD::API_CMD_START;
        apiData[tiBackupApi::API_VAR::API_VAR_BACKUPJOB] = jobName;
        out << apiData;

        apiClient->write(block);
        apiClient->flush();
    }
    else
    {
        qWarning() << apiClient->errorString();
        QMessageBox::warning(this, tr("Error when trying to start backupjob"),
                                    tr("Check if your tiBackup service is running or try to restart."));

        onManualBackupFinished();
    }

    apiClient->close();
    apiClient->disconnect();
    */
}

void MainWindow::ontiBackupLogChanged(__attribute__ ((unused)) const QString &path)
{
    // TODO We can make reading the log more efficient
    tiConfMain main_settings;
    QFile log(QString("%1/tibackup.log").arg(main_settings.getValue("paths/logs").toString()));
    log.open(QIODevice::ReadOnly | QIODevice::Text);
    log.seek((log.size()-10000));

    QString loglines;

    char buf[1024];
    bool skipfirst = true;
    while(!log.atEnd())
    {
        log.readLine(buf, sizeof(buf));
        if(skipfirst == true)
        {
            skipfirst = false;
            continue;
        }

        loglines.append(QString::fromUtf8(buf));
    }

    //QString loglines = QString::fromAscii(buf);

    ui->teTibackupLog->setPlainText(loglines.trimmed());
    QScrollBar *sb = ui->teTibackupLog->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::onManualBackupFinished()
{
    ui->btnStartManualBackup->setEnabled(true);
    ui->statusBar->clearMessage();
}

void MainWindow::onTimeUpdate()
{
    QDateTime curDate = QDateTime::currentDateTime();
    lblTime->setText(curDate.toString("dd.MM.yyyy hh:mm:ss"));
}

void MainWindow::updateServiceStatus()
{
    switch(service->status())
    {
    case tiBackupServiceStatusStarted:
    {
        ui->lblServiceStatus->setText(tr("Started"));
        ui->btnServiceStart->setDisabled(true);
        ui->btnServiceStop->setEnabled(true);
        ui->btnServiceInstall->setDisabled(true);
        break;
    }
    case tiBackupServiceStatusStopped:
    {
        ui->lblServiceStatus->setText(tr("Stopped"));
        ui->btnServiceStart->setEnabled(true);
        ui->btnServiceStop->setDisabled(true);
        ui->btnServiceInstall->setDisabled(true);
        break;
    }
    case tiBackupServiceStatusNotFound:
    {
        ui->lblServiceStatus->setText(tr("init.d script not found"));
        ui->btnServiceStart->setDisabled(true);
        ui->btnServiceStop->setDisabled(true);
        ui->btnServiceInstall->setEnabled(true);
        break;
    }
    case tiBackupServiceStatusFailed:
    case tiBackupServiceStatusUnknown:
    {
        ui->lblServiceStatus->setText(tr("Service failed"));
        ui->btnServiceStart->setDisabled(true);
        ui->btnServiceStop->setDisabled(true);
        ui->btnServiceInstall->setDisabled(true);
        break;
    }
    }

    switch(TiBackupLib::getSystemInitModel())
    {
    case tiBackupInitSystem::Systemd:
        ui->lblServiceStatus->setText(ui->lblServiceStatus->text().append(" (Systemd)"));
        break;
    case tiBackupInitSystem::Other:
        ui->lblServiceStatus->setText(ui->lblServiceStatus->text().append(" (Other initd)"));
        break;
    }
}

void MainWindow::on_btnServiceStart_clicked()
{
    service->start();
}

void MainWindow::on_btnServiceStop_clicked()
{
    service->stop();
}

void MainWindow::on_btnServiceInstall_clicked()
{
    QString initd = main_settings->getValue("paths/initd").toString();
    QString installPath = (!initd.isEmpty()) ? initd : tibackup_config::initd_default;

    int ret = QMessageBox::warning(this, tr("Install tiBackup service"),
                                tr("This will install the tiBackup service in <%1>, continue?").arg(installPath),
                                QMessageBox::Yes | QMessageBox::No);

    switch(ret)
    {
    case QMessageBox::Yes:
    {
        if(service->install(installPath))
        {
            main_settings->setValue("paths/initd", installPath);
            QMessageBox::information(this, tr("Install tiBackup service"), tr("Service was installed successfully."));
        }
        else
        {
            QMessageBox::information(this, tr("Install tiBackup service"), tr("Service was not installed, error."));
        }

        break;
    }
    case QMessageBox::No:
    default:
        return;
    }

    updateServiceStatus();
}
