#ifndef TIBACKUPEDIT_H
#define TIBACKUPEDIT_H

#include <QWidget>

#include "ticonf.h"

namespace Ui {
class tiBackupEdit;
}

class tiBackupEdit : public QWidget
{
    Q_OBJECT

public:
    explicit tiBackupEdit(QWidget *parent = 0, tiBackupJob *job = 0);
    ~tiBackupEdit();

private:
    Ui::tiBackupEdit *ui;

    tiBackupJob *currentJob;

    void updateJobDetails();
};

#endif // TIBACKUPEDIT_H
