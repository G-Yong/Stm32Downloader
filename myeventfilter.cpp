#include "myeventfilter.h"
#include <QDebug>

#ifdef Q_OS_WIN
#include "Windows.h"
#include "Dbt.h"
#endif

MyEventFilter::MyEventFilter(QObject *parent) : QObject(parent)
{

}

bool MyEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if(eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        #ifdef Q_OS_WIN

        MSG* msg = reinterpret_cast<MSG*>(message);
        int msgType = msg->message;
        if(msgType == WM_DEVICECHANGE)
        {
            PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
            switch (msg->wParam) {
            case DBT_DEVICEARRIVAL:
                qDebug() << "DBT_DEVICEARRIVAL:" << lpdb->dbch_devicetype;
                if(lpdb->dbch_devicetype == DBT_DEVTYP_PORT)
                {
                    PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
                    QString portName = QString::fromWCharArray(lpdbv->dbcp_name);
                    qDebug() << "device arrival:" << portName;

                    emit comDevArriaval(portName);
                }
                break;
            case DBT_DEVICEREMOVECOMPLETE:
                qDebug() << "DBT_DEVICEREMOVECOMPLETE:" << lpdb->dbch_devicetype;
                if(lpdb->dbch_devicetype == DBT_DEVTYP_PORT)
                {
                    PDEV_BROADCAST_PORT lpdbv = (PDEV_BROADCAST_PORT)lpdb;
                    QString portName = QString::fromWCharArray(lpdbv->dbcp_name);
                    qDebug() << "device remove complete:" << portName;

                    emit comDevRemoveComplete(portName);
                }
                break;
            case DBT_DEVNODES_CHANGED:
                break;
            default:
                break;
            }
        }

        #endif
    }



    return false;
}
