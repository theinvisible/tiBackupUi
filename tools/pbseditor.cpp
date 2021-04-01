#include "pbseditor.h"
#include "ui_pbseditor.h"

#include <QMessageBox>

#include "ticonf.h"

PBSEditor::PBSEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PBSEditor)
{
    ui->setupUi(this);
}

PBSEditor::~PBSEditor()
{
    delete ui;
}

void PBSEditor::prepareDialog(PBSEditor::ActionType action, const QString &pbs_uuid)
{
    action_type = action;
    this->pbs_uuid = pbs_uuid;

    if(action_type == ActionType_Edit)
    {
        tiConfPBServers *ticonfpb = tiConfPBServers::instance();
        PBServer *p = ticonfpb->getItemByUuid(pbs_uuid);

        ui->leName->setText(p->name);
        ui->leHost->setText(p->host);
        ui->sbPort->setValue(p->port);
        ui->leUsername->setText(p->username);
        ui->lePassword->setText(p->password);
    }
}

void PBSEditor::on_buttonBox_accepted()
{
    if(ui->leName->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Missing information"), tr("You must provide a name!"));
        return;
    }

    if(ui->leHost->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Missing information"), tr("You must provide a host!"));
        return;
    }

    tiConfPBServers *ticonfpb = tiConfPBServers::instance();
    PBServer p;

    if(action_type == ActionType_Edit)
    {
        p.uuid = pbs_uuid;
    }

    p.name = ui->leName->text();
    p.host = ui->leHost->text();
    p.port = ui->sbPort->value();
    p.username = ui->leUsername->text();
    p.password = ui->lePassword->text();
    ticonfpb->saveItem(p);

    emit form_finished();
    done(0);
}

void PBSEditor::on_buttonBox_rejected()
{
    emit form_finished();
}

void PBSEditor::accept()
{

}
