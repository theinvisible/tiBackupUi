#ifndef TASKPLANER_H
#define TASKPLANER_H

#include <QDialog>

namespace Ui {
class TaskPlaner;
}

class TaskPlaner : public QDialog
{
    Q_OBJECT

public:
    explicit TaskPlaner(QWidget *parent = 0);
    ~TaskPlaner();

private:
    Ui::TaskPlaner *ui;
};

#endif // TASKPLANER_H
