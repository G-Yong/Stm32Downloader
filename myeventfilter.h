#ifndef MYEVENTFILTER_H
#define MYEVENTFILTER_H

#include <QObject>
#include <QAbstractNativeEventFilter>

class MyEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit MyEventFilter(QObject *parent = nullptr);

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

signals:
    void comDevArriaval(QString devName);
    void comDevRemoveComplete(QString devName);

};

#endif // MYEVENTFILTER_H
