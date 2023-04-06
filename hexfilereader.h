#ifndef HEXFILEREADER_H
#define HEXFILEREADER_H

#include <QObject>
#include <QFile>
#include <QDir>

#include <QSerialPort>

//https://blog.csdn.net/yx_l128125/article/details/13624947

//第一个字节 表示本行数据的长度；
//第二、三字节表示本行数据的起始地址；

//第四字节表示数据类型，数据类型有：0x00、0x01、0x02、0x03、0x04、0x05。
//'00' Data Rrecord：用来记录数据，HEX文件的大部分记录都是数据记录
//'01' End of File Record:用来标识文件结束，放在文件的最后，标识HEX文件的结尾
//'02' Extended Segment Address Record:用来标识扩展段地址的记录
//'03' Start Segment Address Record:开始段地址记录
//'04' Extended Linear Address Record:用来标识扩展线性地址的记录
//'05' Start Linear Address Record:开始线性地址记录

//然后是数据，最后一个字节 为校验和。
//校验和的算法为：计算校验和前所有16进制码的累加和(不计进位)，检验和 = 0x100 - 累加和
struct DataFrame{
    quint8 length = 0;
    quint32 addr  = 0;
    qint8 type    = 0;
    QByteArray data;
    quint8 checkSum = 0;
};

#define ACK   0x79
#define NACK  0x1f

#pragma execution_character_set("utf-8")

class HexFileReader : public QObject
{
    Q_OBJECT

public:
    explicit HexFileReader(QObject *parent = nullptr);

    // 烧录程序
    Q_INVOKABLE int burnProgram(QString comName, QString fileName);

    static void enterFlash(QSerialPort *port);
    static void enterBootLoader(QSerialPort *port);

private:
    int readFile(QString filePath);

    QList<DataFrame> &dataList();

    int writeData(QSerialPort *port, quint32 addr, QByteArray data);
    int eraseAll(QSerialPort *port);


    static quint16 hexToUInt(QByteArray data);
    static quint8 getCheckSum(QByteArray data);
    static quint8 getXorSum(QByteArray data);

private:
    int decodeData(QString dataStr, DataFrame &df);

signals:
    void progressChanged(int total, int current);

private:
    QList<DataFrame> mDataList;
};

#endif // HEXFILEREADER_H
