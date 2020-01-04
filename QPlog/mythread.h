#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <QString>
#include <QtCore>
#include <string>
using namespace std;

class mythread : public QThread
{
    Q_OBJECT
public:
    explicit mythread(QObject *parent = nullptr);
    void run();
    void plog(void * arg);
    bool Stop;



signals:
    void NumberChanged(int,QString,QString);

public slots:
};

#endif // MYTHREAD_H
