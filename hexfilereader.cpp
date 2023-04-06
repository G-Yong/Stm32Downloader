#include "hexfilereader.h"

#include <QDebug>
#include <QThread>

HexFileReader::HexFileReader(QObject *parent) : QObject(parent)
{
    QThread *thread = new QThread();
    this->moveToThread(thread);
    thread->start();

//    auto a = 0b10101010;
//    auto b = 0b11111111;

//    qDebug() << "binary";
//    qDebug() << "binary:" << QString::number(a ^ b, 2);
}

int HexFileReader::burnProgram(QString comName, QString fileName)
{
    int ret = -1;

    // 解析文件
    ret = readFile(fileName);
    if(ret != 0)
    {
        return -1;
    }

    // 打开串口
    QSerialPort serialPort;
    serialPort.setPortName(comName);
    serialPort.setBaudRate(115200);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::EvenParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    if(serialPort.open(QSerialPort::ReadWrite) == false)
    {
        return -2;
    }

    // 进入isp模式
    enterBootLoader(&serialPort);

    QThread::msleep(100);

    // 发送一个起始位
    uchar data[2] = {0x7f};
    serialPort.write((char*)data, 1);

    // 然后stm32返回ACK(0x79),并进入等待命令模式
    serialPort.waitForReadyRead();
    QByteArray response = serialPort.readAll();
    qDebug() << "response:" << response.toHex();
    if(((uchar)response[0]) == NACK)
    {
        return -3;
    }

    // 此时，单片机已经进入等待命令状态了

    // 擦除
    eraseAll(&serialPort);

    // 将数据一点一点写给单片机（当然，也可以先汇总小于等于256字节的数据，再写入）
    for(int i = 0; i < mDataList.length(); i++)
    {
        DataFrame df = mDataList[i];

        int ret = writeData(&serialPort, df.addr, df.data);
        qDebug() << "burn data:" << QString::number(df.addr, 16) << ret;
        if(ret != 0)
        {
            return -4;
        }

        emit progressChanged(mDataList.length(), i + 1);
    }

    QThread::msleep(1000);

    // 进入正常模式
    qDebug() << "进入正常模式";
    enterFlash(&serialPort);

    return 0;
}

int HexFileReader::readFile(QString filePath)
{
    qDebug() << filePath;

    mDataList.clear();

    QFile file(filePath);
    if(file.open(QFile::ReadOnly))
    {
        quint16 addrBase = 0x0000;

        while (file.atEnd() == false) {
            QString str = file.readLine().trimmed();

            if(str == "")
            {
                continue;
            }

            DataFrame df;
            int ret = decodeData(str, df);
            if(ret != 0)
            {
                return ret;
            }
            if(df.type == 4) // 地址变更
            {
                addrBase = hexToUInt(df.data) & 0xffff;
                continue;
            }
            if(df.type == 5) // 地址变更
            {
                addrBase = df.addr;
                // 据说这个不用写  https://blog.csdn.net/weixin_44057803/article/details/128729894
                continue;
            }
            else if(df.type == 0)
            {
                df.addr += (((addrBase << 16) & 0xffffffff));
            }
            else if(df.type == 1) // 到达末尾
            {
                continue;
            }

//            qDebug() << "data:" << df.type << QString::number(df.addr, 16);

            mDataList.push_back(df);
        }
    }

    return 0;
}

int HexFileReader::writeData(QSerialPort *port, quint32 addr, QByteArray data)
{
    // write memory 命令
    uchar header[] = {0x31, 0xce};
    port->write((char*)header, 2);

//    port->waitForBytesWritten();
    // 等待单片机返回 ACK(0x79) 或者 NACK(0x1F)
    port->waitForReadyRead();
    QByteArray ret = port->readAll();
    if(((uchar)ret[0]) == NACK)
    {
        return -1;
    }

    // 发送地址，从高位到低位
    uchar addrArray[] = {(uchar)((addr >> 24) & 0xff),
                         (uchar)((addr >> 16) & 0xff),
                         (uchar)((addr >> 8)  & 0xff),
                         (uchar)((addr >> 0)  & 0xff)
                        };

    QByteArray cmd((char*)addrArray, 4);
    uchar checkSum = getXorSum(cmd);
    cmd.append(checkSum);

    port->write(cmd);

    // 等待单片机返回 ACK(0x79) 或者 NACK(0x1F)
    port->waitForReadyRead();
    ret = port->readAll();
    if(((uchar)ret[0]) == NACK)
    {
        return -2;
    }

    // 发送数据
    cmd.clear();
    cmd.append(data.length() - 1); // 数据长度
    cmd.append(data); // 数据本身

    checkSum = getXorSum(cmd);
    cmd.append(checkSum);

//    qDebug() << "cmd:" << cmd.toHex();
    port->write(cmd);

    // 等待单片机返回 ACK(0x79) 或者 NACK(0x1F)
    port->waitForReadyRead();
    ret = port->readAll();
    if(((uchar)ret[0]) == NACK)
    {
        return -3;
    }

    return 0;
}

int HexFileReader::eraseAll(QSerialPort *port)
{
    // erase memory 命令
    uchar header[] = {0x43, 0xbc};
    port->write((char*)header, 2);

    // 等待单片机返回 ACK(0x79) 或者 NACK(0x1F)
    port->waitForReadyRead();
    QByteArray ret = port->readAll();
    if(((uchar)ret[0]) == NACK)
    {
        return -1;
    }

    uchar data[] = {0xff, 0x00};
    port->write((char*)data, 2);

    // 等待单片机返回 ACK(0x79) 或者 NACK(0x1F)
    port->waitForReadyRead();
    ret = port->readAll();
    if(((uchar)ret[0]) == NACK)
    {
        return -2;
    }

    return 0;
}

//进入ISP模式
void HexFileReader::enterBootLoader(QSerialPort *port)
{
    //boot0 为1
    //NRST  为0 进入复位
    port->setRequestToSend(true);
    port->setDataTerminalReady(false);

    QThread::msleep(100);

    //boot0 为1
    //NRST  为1 从复位中恢复，并且进入ISP模式
    port->setDataTerminalReady(true);
}

//进入正常模式
void HexFileReader::enterFlash(QSerialPort *port)
{
    //boot0 为1 主要目的是电路上的导通
    //NRST  为0 进入复位
    port->setRequestToSend(true);
    port->setDataTerminalReady(false);

    //boot0 为0; 由于电路上的设计 ，此时实际上除了boot0发生改变之外，NRST也发生了变化
    port->setRequestToSend(false);
//    //NRST 为1 取消复位；此时这个可要可不要
    //    mPort->setDataTerminalReady(true);
}

QList<DataFrame> &HexFileReader::dataList()
{
    return mDataList;
}

quint16 HexFileReader::hexToUInt(QByteArray data)
{
    return (uchar)data[0] << 8 | (uchar)data[1];
}

quint8 HexFileReader::getCheckSum(QByteArray data)
{
    quint8 sum = data[0];

    for(int i = 1; i < data.length(); i++)
    {
        sum += data[i];
    }

    sum = 0x100 - sum;

    return sum;
}

quint8 HexFileReader::getXorSum(QByteArray data)
{
    uchar checkSum = 0x00; //0跟任何数字异或结果是任何数字本身
    for(int i = 0; i < data.length(); i++)
    {
        checkSum ^= data[i];
    }

    return checkSum;
}

int HexFileReader::decodeData(QString dataStr, DataFrame &df)
{
    //先进行字符转16进制
    dataStr = dataStr.replace(":", "");

    auto hexData = QByteArray::fromHex(dataStr.toLatin1());
//    qDebug() << hexData;

    df.length = hexData[0];
    if(hexData.length() != df.length + 5)
    {
        return -1; // 数据长度不对
    }

    df.addr     = hexToUInt(hexData.mid(1, 2));
    df.type     = hexData[3];
    df.data     = hexData.mid(4, df.length);
    df.checkSum = hexData[hexData.length() - 1];

    // 计算校验值是否匹配
    if(df.checkSum != getCheckSum(hexData.left(hexData.length() - 1)))
    {
        return -2;
    }

//    qDebug() << df.length << df.addr << df.checkSum << getCheckSum(hexData.left(hexData.length() - 1));

    return 0;
}
