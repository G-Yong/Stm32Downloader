#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSerialPortInfo>
#include <QDateTime>

#include <QFile>
#include <QDir>
#include <QFileSystemWatcher>
#include <QFileDialog>

#include <QThread>

#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mPort = new QSerialPort();
//    connect(mPort, &QSerialPort::readyRead, this, [=](){
////       qDebug() << mPort->readAll();

//       QString info = QTime::currentTime().toString("hh:mm:ss.zzz -->") + mPort->readAll().toHex(' ');
//       ui->textBrowser->append(info);

//       qDebug() << info << QTime::currentTime();
//    }, Qt::QueuedConnection);
    connect(mPort, &QSerialPort::readyRead, this, &MainWindow::readData, Qt::QueuedConnection);

    connect(ui->pushButton_refresh, &QPushButton::clicked, this, [=](){
        qDebug() << "slot 1";
    }, Qt::QueuedConnection);
    connect(ui->pushButton_refresh, &QPushButton::clicked, [=](){
        qDebug() << "slot 2";
    });
    connect(ui->pushButton_refresh, &QPushButton::clicked, [=](){
        qDebug() << "slot 3";
    });


    auto serialPortList = QSerialPortInfo::availablePorts();

    QStringList portList;
    for(auto info : serialPortList)
    {
        portList << info.portName();
    }

    ui->comboBox_com->clear();
    ui->comboBox_com->addItems(portList);


      // 先通过监视文件夹，发现有新的文件加入之后，再进一步监视文件，这样就可以知道文件何时拷贝完成（理论上是）
//    QFileSystemWatcher *mFileWatcher = new QFileSystemWatcher();
//    mFileWatcher->addPath("C:/Users/Administrator/Desktop/darknet/123");
//    connect(mFileWatcher, &QFileSystemWatcher::fileChanged, [=](const QString &path){
//        qDebug() << "fileChanged" << path;
//    });
//    connect(mFileWatcher, &QFileSystemWatcher::directoryChanged, [=](const QString &path){
//        qDebug() << "directoryChanged" << path;
//    });

    mHexfileReader = new HexFileReader();
    connect(mHexfileReader, &HexFileReader::progressChanged, this, [=](int total, int current){
        ui->progressBar->setValue((double)current / (double)total * 100.0);
    }, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readData()
{
    qDebug() << "read data thread:" << QThread::currentThread();

    QString info = "";
//    info = QTime::currentTime().toString("hh:mm:ss.zzz -->") + mPort->readAll().toHex(' ');
//    ui->textBrowser->append(info);

    qDebug() << info << QTime::currentTime();
}

void MainWindow::on_pushButton_refresh_clicked()
{
    QStringList portList;
    auto serialPortList = QSerialPortInfo::availablePorts();
    foreach(auto info, serialPortList)
    {
        portList << info.portName();
    }

    ui->comboBox_com->clear();
    ui->comboBox_com->addItems(portList);
}

void MainWindow::on_pushButton_connect_clicked()
{
    if(ui->pushButton_connect->text() == "连接")
    {
        mPort->setPortName(ui->comboBox_com->currentText());
        mPort->setBaudRate(115200);
        mPort->setParity(QSerialPort::EvenParity);
        mPort->setDataBits(QSerialPort::Data8);
        mPort->setStopBits(QSerialPort::OneStop);

        if(mPort->open(QSerialPort::ReadWrite))
        {
            ui->pushButton_connect->setText("断开");
        }

    }
    else
    {
        mPort->close();

        ui->pushButton_connect->setText("连接");
    }

}

void MainWindow::on_checkBox_DTR_clicked(bool checked)
{
    qDebug() << "set DTR:" << mPort->setDataTerminalReady(checked);
}

void MainWindow::on_checkBox_RTS_clicked(bool checked)
{
    qDebug() << "set RTS:" << mPort->setRequestToSend(checked);
}



void MainWindow::on_pushButton_clear_clicked()
{
    ui->textBrowser->clear();
}

void MainWindow::on_pushButton_bootLoader_clicked()
{
}

void MainWindow::on_pushButton_flash_clicked()
{
}

void MainWindow::on_pushButton_write_clicked()
{
    auto hexData = QByteArray::fromHex(ui->lineEdit_data->text().toLatin1());

    qDebug() << "direct write:" <<  mPort->write(hexData);
    qDebug() << "bytea available:" << QTime::currentTime() <<  mPort->bytesAvailable();
    qDebug() << "wait for ready read:" << mPort->waitForReadyRead(5000);
}

void MainWindow::on_pushButton_writeFlash_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择程序文件"),
                                                    "/home",
                                                    tr("hex (*.hex)"));

    ui->textBrowser->append(fileName);

    QMetaObject::invokeMethod(mHexfileReader, "burnProgram", Qt::QueuedConnection,
                              Q_ARG(QString, ui->comboBox_com->currentText()),
                              Q_ARG(QString, fileName));
}
