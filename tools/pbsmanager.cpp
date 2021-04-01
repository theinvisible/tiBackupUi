#include "pbsmanager.h"
#include "ui_pbsmanager.h"

#include <QStandardItemModel>
#include <QMessageBox>

#include "ticonf.h"
#include "pbseditor.h"

PBSManager::PBSManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PBSManager)
{
    ui->setupUi(this);

    QStringList headers;
    headers << tr("Name") << tr("Host") << tr("Username") << tr("Uuid");
    QStandardItemModel *model = new QStandardItemModel(ui->tvPBServer);
    model->setHorizontalHeaderLabels(headers);
    ui->tvPBServer->setModel(model);
    ui->tvPBServer->sortByColumn(0, Qt::AscendingOrder);

    ui->tvPBServer->header()->resizeSection(0, 250);
    ui->tvPBServer->header()->resizeSection(1, 250);
    ui->tvPBServer->header()->resizeSection(2, 300);

    loadData();
}

PBSManager::~PBSManager()
{
    delete ui;
}

void PBSManager::on_buttonBox_accepted()
{
    emit form_finished();

    done(0);
}

void PBSManager::on_buttonBox_rejected()
{
    emit form_finished();
}

void PBSManager::accept()
{

}

void PBSManager::loadData()
{
    tiConfPBServers *ticonfpbs = tiConfPBServers::instance();
    ticonfpbs->readItems();

    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvPBServer->model());
    model->removeRows(0, model->rowCount());

    QStandardItem *item = 0;
    QStandardItem *item2 = 0;
    QStandardItem *item3 = 0;
    QStandardItem *item4 = 0;

    QList<PBServer*> pbs = ticonfpbs->getItems();
    for(int i=0; i < pbs.count(); i++)
    {
        PBServer *pb = pbs.at(i);

        item = new QStandardItem(pb->name);
        item->setData(pb->uuid);
        item2 = new QStandardItem(pb->host);
        item3 = new QStandardItem(pb->username);
        item4 = new QStandardItem(pb->uuid);

        model->setItem(i, 0, item);
        model->setItem(i, 1, item2);
        model->setItem(i, 2, item3);
        model->setItem(i, 3, item4);
    }

    ui->tvPBServer->setSortingEnabled(true);
    ui->tvPBServer->sortByColumn(0, Qt::AscendingOrder);
}

void PBSManager::on_btnAdd_clicked()
{
    PBSEditor *d = new PBSEditor(this);
    d->prepareDialog(PBSEditor::ActionType_Add);
    connect(d, &PBSEditor::form_finished, this, [=]() { loadData(); });
    d->show();
}

void PBSManager::on_tvPBServer_doubleClicked(const QModelIndex &index)
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvPBServer->model());
    QItemSelectionModel *selmodel = ui->tvPBServer->selectionModel();
    QModelIndexList sellist = selmodel->selectedRows(0);

    if(sellist.count() < 1)
    {
        return;
    }

    QString uuid = model->itemFromIndex(sellist.at(0))->data().toString();
    if(uuid.isEmpty())
        return;

    PBSEditor *d = new PBSEditor(this);
    d->prepareDialog(PBSEditor::ActionType_Edit, uuid);
    connect(d, &PBSEditor::form_finished, this, [=]() { loadData(); });
    d->show();
}

void PBSManager::on_btnRemove_clicked()
{
    QStandardItemModel *model = dynamic_cast<QStandardItemModel *>(ui->tvPBServer->model());
    QItemSelectionModel *selmodel = ui->tvPBServer->selectionModel();
    QModelIndexList sellist = selmodel->selectedRows(0);

    if(sellist.count() < 1)
    {
        return;
    }

    QString pbUuid = model->itemFromIndex(sellist.at(0))->data().toString();
    if(pbUuid.isEmpty())
        return;

    QMessageBox::StandardButton sel = QMessageBox::question(this, tr("Remove PBS item"), tr("You really want to remove PBS Server %1").arg(pbUuid));
    if(sel == QMessageBox::Yes)
    {
        tiConfPBServers *ticonfpbs = tiConfPBServers::instance();
        ticonfpbs->removeItemByUuid(pbUuid);
        loadData();
    }
}
