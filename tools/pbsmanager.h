#ifndef PBSMANAGER_H
#define PBSMANAGER_H

#include <QDialog>

namespace Ui {
class PBSManager;
}

class PBSManager : public QDialog
{
    Q_OBJECT

public:
    explicit PBSManager(QWidget *parent = nullptr);
    ~PBSManager();

    enum ActionType
    {
        ActionType_Add,
        ActionType_Edit
    };

signals:
    void form_finished();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_btnAdd_clicked();
    void on_tvPBServer_doubleClicked(const QModelIndex &index);
    void on_btnRemove_clicked();

public slots:
    void accept();
    void loadData();

private:
    Ui::PBSManager *ui;
    ActionType action_type;
    QString pbserver_name;
};

#endif // PBSMANAGER_H
