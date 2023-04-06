#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

#include "myeventfilter.h"

int isBig()
{
    QByteArray ba;
    ba.push_back(0x22);
    ba.push_back(0x11);

    quint16 *num = (quint16*)ba.data();
    qDebug() << *num << 0x1122;


    int a = 0x12345678;
    char b = *(char*)&a;

    for(int i = 0; i < 4; i++)
    {
        qDebug() << QString::number(*((uchar *)&a + i), 16);
    }

    if (0x12 == b)
    {
        return 0;
    }

    return -1;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MyEventFilter evtFilter;
    a.installNativeEventFilter(&evtFilter);

    qDebug() << "is big:" << isBig();

    MainWindow w;
    w.show();
    return a.exec();
}
