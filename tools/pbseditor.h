#ifndef PBSEDITOR_H
#define PBSEDITOR_H

#include <QDialog>

namespace Ui {
class PBSEditor;
}

class PBSEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PBSEditor(QWidget *parent = nullptr);
    ~PBSEditor();

    enum ActionType
    {
        ActionType_Add,
        ActionType_Edit
    };

    void prepareDialog(ActionType action, const QString &pbs_uuid = "");

signals:
    void form_finished();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void on_btnTest_clicked();

public slots:
    void accept();

private:
    Ui::PBSEditor *ui;
    ActionType action_type;
    QString pbs_uuid;
};

#endif // PBSEDITOR_H
