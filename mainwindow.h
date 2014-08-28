#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "tibackupadd.h"
#include "tibackupedit.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnAddBackup_clicked();
    void onjobAdded(tiBackupJob job);

    void onActionAbout();

    void on_btnBackupJobDelete_clicked();

    void on_btnBackupJobEdit_clicked();

private:
    Ui::MainWindow *ui;

    void refreshBackupJobList();
};

#endif // MAINWINDOW_H
