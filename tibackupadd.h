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

private:
    Ui::tiBackupAdd *ui;
};

#endif // TIBACKUPADD_H
