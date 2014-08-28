#include "tibackupedit.h"
#include "ui_tibackupedit.h"

tiBackupEdit::tiBackupEdit(QWidget *parent, tiBackupJob *job) :
    QWidget(parent),
    ui(new Ui::tiBackupEdit)
{
    ui->setupUi(this);
    currentJob = job;

    updateJobDetails();
}

tiBackupEdit::~tiBackupEdit()
{
    delete ui;
    delete currentJob;
}

void tiBackupEdit::updateJobDetails()
{
    ui->leBackupJobName->setText(currentJob->name);
}
