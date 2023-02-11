#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <iostream>
#include <QDebug>
#include <QThread>
#include <QFileDialog>
#include "imgconvert.h"

#define BitImgPath "./AllFrame.txt"  //存总帧数
using namespace cv;
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->port->setText("6789");
    setWindowTitle("ESP32OLED播放器");
    ui->progressBar->setValue(0);
    updateLocalFrameNum(0);
    invColor = false;

    m_server = new QTcpServer(this);    // 创建 QTcpServer 对象
    // 检测是否有新的客户端连接
    connect(m_server, &QTcpServer::newConnection, this, [=]()
    {
        m_tcp = m_server->nextPendingConnection();
        ui->record->append("成功和客户端建立了新的连接...");
        m_tcp->write(QByteArray::number(resizeWidth, 10));
        // 检测是否有客户端数据
        connect(m_tcp, &QTcpSocket::readyRead, this, [=]()
        {
            // 接收数据
            QString recvMsg = m_tcp->readAll();
            ui->record->append("收到 ->"+recvMsg);
        });
        // 客户端断开了连接
        connect(m_tcp, &QTcpSocket::disconnected, this, [=]()
        {
            ui->record->append("客户端已经断开了连接...");
            m_tcp->deleteLater();
        });
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_selectVideo_clicked()
{
    QString filePath = QFileDialog::getOpenFileName();
    if(filePath.isEmpty()){
        QMessageBox::warning(this, "打开文件", "选择路径不能为空");
        return;
    }
    ui->filePath->setText(filePath);
}

void MainWindow::on_sendVideo_clicked()  //发送文件
{
    ui->progressBar->setValue(0);
    double sendNum = 0;
    double sendRatio = 100 / (double)maxFrame;
    if(resizeWidth%8 != 0){
        resizeWidth += (8 - resizeWidth%8);
    }
    int bufSize = resizeWidth*64+5;
    for (int j = 0;j<maxFrame;j++) {
        //一行是一帧
        QString fileName = QString("./bitmapSendCache/BitFram_%1.txt").arg(j,8,10, QLatin1Char('0'));
        QFile file(fileName);
        if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
            ui->record->append("文件打开失败");
        }
        char * str = new char[bufSize];
        qint64 readNum = file.readLine(str,bufSize);
        if((readNum !=0) && (readNum != -1))
        {
            QString readyToConvert(str);
            QString readyToCompress;
            this->bitStringConvert(readyToConvert);
            this->compressBitString(readyToConvert, readyToCompress);
            QByteArray trytry = readyToCompress.toUtf8();
            char*  readyToSend = trytry.data();
            m_tcp->write(readyToSend);
            m_tcp->write("#");
            sendNum++;
            ui->progressBar->setValue((int)sendNum*sendRatio);
        }
        file.close();
    }
    m_tcp->write("**");
    ui->progressBar->setValue(100);
    ui->record->append("发送完毕");
}

void MainWindow::bitStringConvert(QString &inPut)
{
    for (int i = 0; i < inPut.length(); i++)
    {
        if (inPut[i] == 48)
        {
            inPut[i] = 'a';
        }
        else if(inPut[i] == 49)
        {
            inPut[i] = 'b';
        }
    }
}

void MainWindow::compressBitString(QString inPut, QString &outPut)
{
    QChar nextVal;
    QChar temp;
    int reptAccum;
    int offset;
    for (int pose = 0; pose < inPut.length();)
    {
        reptAccum = 0;
        offset = 1;
        temp = inPut[pose];             // 取一个首字符
        outPut += temp;                        // 记录到输出串
        nextVal = inPut[pose + offset]; // 下一个值
        while (nextVal == temp)
        {
            reptAccum++;
            offset++;
            if((pose + offset) < inPut.length()){
                nextVal = inPut[pose + offset];  //当心越界
            }else{
                break;
            }
        }
        pose += offset;
        if (reptAccum > 0)
        {
            outPut += QString::number(reptAccum, 10);
        }
    }
}

void MainWindow::on_startListen_clicked()
{

    unsigned short port = ui->port->text().toUShort();
    m_server->listen(QHostAddress::Any, port);
    ui->startListen->setDisabled(true);

}

void MainWindow::updateLocalFrameNum(bool WR)  //ture 为写 false 为读
{
    QFile file(BitImgPath);
    if(WR){
        if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text)){
            ui->record->append("文件打开失败");
        }
        file.write(QByteArray::number(maxFrame, 10));
        file.write("\n");
        file.write(QByteArray::number(resizeWidth, 10));
    }else{
        if(!file.open(QIODevice::ReadWrite|QIODevice::Text)){
            ui->record->append("文件打开失败");
        }
        maxFrame = 0;
        resizeWidth = 0;
        char * str = new char[10];
        int read[2] = {0};
        for (int n = 0; n < 2; n++) {
            qint64 readNum = file.readLine(str,10);
            if((readNum !=0) && (readNum != -1))
            {
                for(int i = 0; i < readNum; i++){
                    if(str[i] >= 48 && str[i] <= 57){
                        read[n] *= 10;
                        read[n] += (str[i]-48);
                    }
                }
            }
        }
        maxFrame = read[0];
        resizeWidth = read[1];
        ui->record->append("读取到"+QString::number(maxFrame,10)+"帧");
        ui->record->append("读取到宽度"+QString::number(resizeWidth,10));
    }
    file.close();
}

void MainWindow::appendVideoInfo(QString FAF)
{
    QStringList FAFlist = FAF.split("#");
    ui->record->append("总帧数"+FAFlist[0]);
    ui->record->append("帧率"+FAFlist[1]);
}

/*------反色-------*/
void MainWindow::on_convertBW_stateChanged(int arg1)
{
    invColor = (arg1 == Qt::Checked)?true:false;
}

void MainWindow::on_convertVedio_clicked()
{
//    qDebug() << "主线程对象的地址: " << QThread::currentThread();
    /*------准备-------*/
    //其实不用频繁创建和释放，线程没活会处于yield状态不占CPU
    QThread* sub = new QThread;
    ImgConvert* imgConvTaskObj = new ImgConvert;
    imgConvTaskObj->moveToThread(sub);    // 移动到子线程中工作
    sub->start();
    /*------转换完成-------*/
    connect(imgConvTaskObj, &ImgConvert::isDone, this, [=](int raelFramNum){
        maxFrame = raelFramNum;
        ui->progressBar->setValue(100);
        ui->record->append("处理完毕!");

        sub->quit();
        sub->wait();
        sub->deleteLater();
        imgConvTaskObj->deleteLater();
        this->updateLocalFrameNum(1);
    });
    /*------转换进度-------*/
    connect(imgConvTaskObj, &ImgConvert::processVal, this, [=](int pVal){
        ui->progressBar->setValue(pVal);
    });
    /*------转换Cache文件夹-------*/
    QDir *dir = new QDir;
    if(dir->exists("./bitmapSendCache")){
        QDir del("./bitmapSendCache");
        del.removeRecursively();
    }
    if(dir->mkpath("./bitmapSendCache")){
        ui->record->append("临时缓存文件夹创建成功!");
    }
    /*------接收传出信息-------*/
    connect(imgConvTaskObj, &ImgConvert::frameAndFps, this, &MainWindow::appendVideoInfo);
    connect(imgConvTaskObj, &ImgConvert::reWidth, this, [=](int rW){
        resizeWidth = rW;
    });

    connect(this, &MainWindow::convPrepareDone, imgConvTaskObj, &ImgConvert::work);
    emit convPrepareDone(ui->filePath->text(), invColor);
}
