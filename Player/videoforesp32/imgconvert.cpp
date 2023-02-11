#include "imgconvert.h"
#include <QDebug>
#include <QFile>
#include <QThread>
using namespace cv;

ImgConvert::ImgConvert(QObject *parent) : QObject(parent)
{
    invertColor = false;
}

void ImgConvert::gray_save2file(Mat rowImg, int num){
        /*------------调整大小--------------*/
        double imgWidth = rowImg.cols;
        double imgHigh = rowImg.rows;
        double resizeRatio = imgHigh/64;  //最大限度在屏幕上放下图像
        int outWidth = (int)(imgWidth/resizeRatio);
        Size theReSize = Size(outWidth, 64);  //得到目标图像尺寸
        emit reWidth(outWidth);
        Mat afterResizeImg;
        cv::resize(rowImg,afterResizeImg,theReSize,0, 0, INTER_AREA);
        /*------------灰度处理--------------*/
        Mat grayImg;
        cv::cvtColor(afterResizeImg, grayImg, COLOR_BGR2GRAY);
        /*------------二值化--------------*/
        Mat binImg;
        if(invertColor){
            cv::threshold(grayImg, binImg, 100, 255, THRESH_BINARY_INV);  //此处应该作个控件调节二值化效果
        }else{
            cv::threshold(grayImg, binImg, 100, 255, THRESH_BINARY);  //此处应该作个控件调节二值化效果
        }
        /*------------取值转换--------------*/
        QByteArray temp;
        int tempPixVal;
        QString oneGroup;  //补齐8位
        QString fileName = QString("./bitmapSendCache/BitFram_%1.txt").arg(num,8,10, QLatin1Char('0')); //一行是一帧
        // 使用以上三个变量拼接一个动态字符串
        QFile* file = new QFile(fileName);
        file->open(QFile::WriteOnly);
        for (int i = 0;i<theReSize.height;i++)
        {
            oneGroup="";  //一行一行写
            for (int n = 0;n < theReSize.width;n++)
            {
                tempPixVal = binImg.at<uchar>(i,n);
                if(tempPixVal == 255)
                {
                    temp = "0";
                    oneGroup += "0";
                }else
                {
                    temp = "1";
                    oneGroup += "1";
                }
                file->write(temp);
            }
            if(oneGroup.size()%8 != 0)  //补齐到8的倍数
            {
                int times = 8 - (oneGroup.size()%8);
                for(int i = 0; i <times; i++)
                {
                    oneGroup += "0";
                    file->write("0");
                }
            }
        }
        file->close();
        file->deleteLater();
}

void ImgConvert::work(QString videoPath, bool invsColor)
{
    invertColor = invsColor;
    int fileNum = 0;                // 该变量表示当前文件的编号
    int pVal = 1;
    VideoCapture cap(videoPath.toStdString());
    int frame_count = cap.get(CAP_PROP_FRAME_COUNT);
    double fps = cap.get(CAP_PROP_FPS);
    QString frameNum = QString::number(frame_count, 10)+"#"+QString::number((int)fps, 10);
    emit frameAndFps(frameNum);
    double processRatio =  100 / (double)frame_count;
    Mat frame;
    while (true) {
        cap.read(frame);//frame为输出，read是将捕获到的视频一帧一帧的传入frame
        //对视频读取时，同图像一样会有判空操作
        if (frame.empty()) {
            break;
        }
        gray_save2file(frame, fileNum);
        fileNum++;
        maxFrame = fileNum;
        pVal = fileNum*processRatio;
        emit processVal(pVal);
    }
    cap.release();
    emit isDone(maxFrame);
}
