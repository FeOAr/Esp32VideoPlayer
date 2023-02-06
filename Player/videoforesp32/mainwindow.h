#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QLabel>
#include <QTcpServer>
#include <QTcpSocket>
#include <opencv2/opencv.hpp>
#include <QQueue>

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
    void on_selectVideo_clicked();

    void on_sendVideo_clicked();

    void on_startListen_clicked();

    void updateLocalFrameNum(bool);
    void appendVideoInfo(QString);

    void on_convertBW_stateChanged(int arg1);

private:
    void gray_save2file(cv::Mat, int);
    void bitStringConvert(QString&);  //01 -> ab
    void compressBitString(QString, QString&); //压缩
    QQueue<QString> fileCache;
signals:
    void inverseColor(bool);

private:
    Ui::MainWindow *ui;
    QTcpServer* m_server;
    QTcpSocket* m_tcp;
    QLabel* m_statusBar;
    QString fileName;
    bool sendFlag;
    int maxFrame;
    int resizeWidth;
};
#endif // MAINWINDOW_H

