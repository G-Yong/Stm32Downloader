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

    auto serialPortList = QSerialPortInfo::availablePorts();

    QStringList portList;
    foreach (auto info, serialPortList)
    {
        portList << info.portName();
    }

    ui->comboBox_com->clear();
    ui->comboBox_com->addItems(portList);

    mHexfileReader = new HexFileReader();
    connect(mHexfileReader, &HexFileReader::progressChanged, this, [=](int total, int current){
        ui->progressBar->setValue((double)current / (double)total * 100.0);
    }, Qt::QueuedConnection);
}

MainWindow::~MainWindow()
{
    delete ui;
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


void MainWindow::on_pushButton_clear_clicked()
{
    ui->textBrowser->clear();
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
