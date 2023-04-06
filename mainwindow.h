#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

#include "hexfilereader.h"

#pragma execution_character_set("utf-8")

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_pushButton_refresh_clicked();

    void on_pushButton_clear_clicked();


    void on_pushButton_writeFlash_clicked();

private:
    Ui::MainWindow *ui;

    HexFileReader *mHexfileReader;
};
#endif // MAINWINDOW_H
