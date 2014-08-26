#ifndef TIBACKUPADD_H
#define TIBACKUPADD_H

#include <QWidget>

#include "tibackuplib.h"

namespace Ui {
class tiBackupAdd;
}

class tiBackupAdd : public QWidget
{
    Q_OBJECT

public:
    explicit tiBackupAdd(QWidget *parent = 0);
    ~tiBackupAdd();

private slots:
    void on_comboBackupDevice_currentIndexChanged(int index);

    void on_comboBackupPartition_currentIndexChanged(int index);

    void on_btnSelectSource_clicked();

    void on_btnSelectDest_clicked();

    void on_btnAddBackupFolder_clicked();

    void on_btnRemoveBackupFolder_clicked();

    void on_btnAddBackupJob_clicked();

private:
    Ui::tiBackupAdd *ui;
};

#endif // TIBACKUPADD_H