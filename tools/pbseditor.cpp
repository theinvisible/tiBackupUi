#include "pbseditor.h"
#include "ui_pbseditor.h"

#include <QMessageBox>
#include <QFileDialog>

#include "ticonf.h"
#include "pbsclient.h"

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
        ui->leFingerprint->setText(p->fingerprint);
        ui->leKeyFile->setText(p->keyfile);
        ui->leKeyPass->setText(p->keypass);
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
    p.fingerprint = ui->leFingerprint->text();
    p.keyfile = ui->leKeyFile->text();
    p.keypass = ui->leKeyPass->text();
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

void PBSEditor::on_btnTest_clicked()
{
    pbsClient *pbs = pbsClient::instanceUnique();
    HttpStatus::Code status = pbs->auth(ui->leHost->text(), ui->sbPort->value(), ui->leUsername->text(), ui->lePassword->text());
    if(status == HttpStatus::Code::OK)
    {
        QMessageBox::information(this, tr("PBS Connection Test"), tr("Connection Test OK!"));
    }
    else
    {
        QMessageBox::warning(this, tr("PBS Connection Test"), tr("Connection Test failed!"));
    }
}

void PBSEditor::on_btnSelectKeyFile_clicked()
{
    QString startDir = (ui->leKeyFile->text().isEmpty()) ? "/" : ui->leKeyFile->text();

    QString file = QFileDialog::getOpenFileName(this, tr("Choose the key encryption file"), startDir);

    if(!file.isEmpty())
        ui->leKeyFile->setText(file);
}
