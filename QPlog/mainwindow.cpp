#include <QTreeWidgetItem>
#include <QtCore>
#include <QtGui>
#include <QMap>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mythread.h"
#include "header.h"


#include <iostream>
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <time.h>


#include <map>
#include <QMessageBox>

static QMap<int,qmap>qm;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mThread = new mythread(this);
    connect(mThread,SIGNAL(NumberChanged(int,QString,QString,QString)),this,SLOT(onNumberChanged(int,QString,QString,QString)));
    connect(this,SIGNAL(signal_mapclear()),mThread,SLOT(slot_mapclear()));
    mythread proccess;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNumberChanged(int Number,QString host,QString cookie, QString basedomain)
{
    struct qmap * qm_data =new struct qmap;
    QTreeWidgetItem *itm=new QTreeWidgetItem(ui->treeWidget);

    qm_data->host=host;
    qm_data->cookie=cookie;
    qm_data->base_domain=basedomain;
    qm[Number]=*qm_data;

    auto it = qm.find(Number);
//    QMessageBox::information(this,host,cookie);
//    itm->setText(0,QString::number(Number));
//    itm->setText(1,basedomain);
//    itm->setText(2,host);
//    itm->setText(3,cookie);

    itm->setText(0,QString::number(it.key()));
    itm->setText(1,it.value().base_domain);
    itm->setText(2,it.value().host);
    itm->setText(3,it.value().cookie);
    ui->treeWidget->addTopLevelItem(itm);



}

void MainWindow::on_actionMonitor_Mode_triggered()
{
    //system("")
}

void MainWindow::on_Start_clicked()
{
    if(mThread->Stop)
    {
//        ui->statusbar->showMessage("시작!",6);
        QMessageBox::information(this,"시작","시작");
        mThread->start();
        mThread->Stop=false;
    }
    else
    {
        QMessageBox::information(this,"Warring","현재 실행중 입니다.");
    }
}

void MainWindow::on_Stop_clicked()
{
    mThread->Stop=true;
    mThread->wait();
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    int i;
    for(i=0;i<argc;i++)
    {
        printf("%s = %s\n",azColName[i],argv[i] ? argv[i]:"NULL");
    }
    printf("\n");
    return 0;
}

void sql_exec(sqlite3 *db,char *zErrMsg,char *sql,string str_sql)
{
    zErrMsg=static_cast<char*>(malloc(1));
    sql=static_cast<char*>(malloc(sizeof(char*)));
    int rc;
    vector<char>writable(str_sql.begin(),str_sql.end());        //string to char* = vecter
    writable.push_back('\0');
    sql = &writable[0];
    cout<<sql<<'\n';
    rc = sqlite3_exec(db,sql,callback,nullptr,&zErrMsg);

    if(rc!= SQLITE_OK){
        fprintf(stderr,"SQL error: %s\n",zErrMsg);
    }else{
        fprintf(stdout,"Successfull\n");
    }
}

string erase_space(std::string str)
{
    char *str_buff = static_cast<char*>(malloc(1024));
    strcpy(str_buff,str.c_str());
    string data;
//    cout<<str_buff<<'\n';
    char *tok= strtok(str_buff," \r\n");
    while(true){
        string str1(tok);
        if(tok!=nullptr)
        {
            data = str1;
            break;
        }
        tok=strtok(nullptr," \r\n");
    }
    return data;
}
void sqlite(int id)
{
    sqlite3 *db;
    char *zErrMsg=static_cast<char*>(malloc(1));
    int rc;
    string str_sql,str_sql2;
    char *sql=static_cast<char*>(malloc(sizeof(char*)));
    rc= sqlite3_open("/root/.mozilla/firefox/o0e4u1b1.default/cookies.sqlite",&db);

    if(rc){
        fprintf(stderr, "Can't open %s\n",sqlite3_errmsg(db));
    } else{
        fprintf(stderr,"Open db Successfully\n");
    }

    string sql_delete = "DELETE FROM moz_cookies;";
    sql_exec(db,zErrMsg,sql,sql_delete);

    time_t result=time(nullptr);
    result=result+ 365*24*60*60;
    string expiry=to_string(result);
    string lastAccessed,creationTime = to_string(time(nullptr));

    auto it = qm.find(id);
    if(it!=qm.end())
    {
        string host = it.value().host.toStdString();        //QString Host -> string Host
        string cookie = it.value().cookie.toStdString();
        string base_Domain = it.value().base_domain.toStdString();
        string cookie_name;
        string cookie_value;

        host = erase_space(host);
        base_Domain = erase_space(base_Domain);

        char *str_buff = static_cast<char*>(malloc(1500));
        strcpy(str_buff,cookie.c_str());
        //    cout<<str_buff<<'\n';
        char *tok= strtok(str_buff,";");
        int i=1;
        while(tok!=nullptr){
            string number = to_string(i);
            string str(tok);

            string err;
            cookie_name = str.substr(0,str.find_first_of("="));
            cookie_value = str.substr(str.find_first_of("=")+1);

            while(true){
                err = cookie_value.substr(0,1);
                if(err ==" ") cookie_value=cookie_value.substr(1);  //맨 앞의 공백 제거.
                else{
                    err = cookie_value.substr(cookie_value.length()-1);     //맨뒤로 이동
                    if(err == " ")
                        cookie_value=cookie_value.substr(0,cookie_value.length()-1);
                    else if(err == "\n")
                        cookie_value=cookie_value.substr(0,cookie_value.length()-1);
                    else
                        break;
                }
            }
            while(true){
                err = cookie_name.substr(0,1);
                if(err ==" ") cookie_name=cookie_name.substr(1);  //맨 앞의 공백 제거.
                else{
                    err = cookie_name.substr(cookie_name.length()-1);     //맨뒤로 이동
                    if(err == " ")
                        cookie_name=cookie_name.substr(0,cookie_name.length()-1);
                    else
                        break;
                }
            }

//            err = cookie_name.substr(0,1);
//            if(err ==" ") cookie_name=cookie_name.substr(1);

            str_sql="INSERT OR REPLACE INTO moz_cookies(id,baseDomain,name,value,host,path,expiry)"\
                    "VALUES("+number+",'"+base_Domain+"','"+cookie_name+"','"+cookie_value+"','."+base_Domain+"','/',"+expiry+");";
            tok=strtok(nullptr,";");
            i++;

            //    str_sql="DELETE FROM moz_cookies;";

            sql_exec(db,zErrMsg,sql,str_sql);
        }
    }
    sqlite3_close(db);
}

void MainWindow::on_treeWidget_doubleClicked(const QModelIndex &index)
{
    QString link="naver.com";
    int key = index.row()+1;
    auto it =qm.find(key);
    if(it != qm.end())
    {
        QMessageBox::information(this,"접속!",it.value().host,"YES");
        sqlite(key);
    }
//    QMessageBox::information(this,"TEST",QString::number(key));

    string front_str="firefox --new-window ";
    system(front_str.append(link.toStdString()).c_str());
}

void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    auto it = qm.find(column);
    QMessageBox::information(this,"TEST",it.value().cookie);
}

void MainWindow::on_pushButton_clicked()
{
    qm.clear();
    ui->treeWidget->clear();
}
