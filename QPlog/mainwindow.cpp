#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mythread.h"
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QtCore>
#include <QtGui>
#include <QDesktopServices>
#include "header.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mThread = new mythread(this);
    connect(mThread,SIGNAL(NumberChanged(int,QString,QString)),this,SLOT(onNumberChanged(int,QString,QString)));
    mythread proccess;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNumberChanged(int Number,QString host,QString cookie)
{
    QTreeWidgetItem *itm=new QTreeWidgetItem(ui->treeWidget);
    itm->setText(0,QString::number(Number));
    itm->setText(1,host);
    itm->setText(2,cookie);
    ui->treeWidget->addTopLevelItem(itm);
}

void MainWindow::on_actionMonitor_Mode_triggered()
{
    //system("")
}

void MainWindow::on_Start_clicked()
{
    mThread->start();
    mThread->Stop=false;
}

void MainWindow::on_Clear_clicked()
{
    ui->treeWidget->clear();
}

void MainWindow::on_Stop_clicked()
{
    mThread->Stop=true;
}



void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QString link="naver.com";
    string front_str="firefox --new-window ";
    system(front_str.append(link.toStdString()).c_str());
}
