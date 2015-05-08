#include "taskplaner.h"
#include "ui_taskplaner.h"

TaskPlaner::TaskPlaner(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskPlaner)
{
    ui->setupUi(this);
}

TaskPlaner::~TaskPlaner()
{
    delete ui;
}
