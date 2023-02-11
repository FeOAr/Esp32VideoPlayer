#ifndef IMGCONVERT_H
#define IMGCONVERT_H

#include <QObject>
#include <opencv2/opencv.hpp>

class ImgConvert : public QObject
{
    Q_OBJECT
public:
    explicit ImgConvert(QObject *parent = nullptr);
    void gray_save2file(cv::Mat, int);
    void work(QString, bool);  //自线程，负责图转bit.txt
public:
    int maxFrame;
    bool invertColor;

signals:
    void isDone(int);  //处理完成信号
    void processVal(int);  //进度条
    void frameAndFps(QString);  //返回一下帧数和帧率
    void reWidth(int);

};

#endif // IMGCONVERT_H
