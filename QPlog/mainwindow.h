#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include <QTreeWidgetItem>
#include <QString>
#include "mythread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    mythread *mThread;

private:
    Ui::MainWindow *ui;

public slots:
    void onNumberChanged(int,QString,QString,QString);

private slots:
    void on_actionMonitor_Mode_triggered();
    void on_Start_clicked();
    void on_Stop_clicked();
    void on_treeWidget_doubleClicked(const QModelIndex &index);
    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_pushButton_clicked();
};
#endif // MAINWINDOW_H
