#include "HardwareSerial.h"
#include "esp32-hal.h"
#include <sys/_types.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>
using namespace std;

const char hexMap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

unsigned char binToHexDeal8Bit(String rawStr) {
  string sub;
  for (int i = 7; i >= 0; i--) {  //每字节内比特序列大小端转换
    sub += rawStr[i];
  }
  string sub_high = sub.substr(0, 4);  // 3-0
  string sub_low = sub.substr(4, 4);   // 7-4
  int highDec;                         //高位四个bit转十进制
  int lowDec;                          //低位四个bit转十进制
  string finalHex;                     //最终8位bit输出一个十六进制字符串

  highDec = (sub_high[0] - 48) * 8 + (sub_high[1] - 48) * 4 + (sub_high[2] - 48) * 2 + (sub_high[3] - 48);  //8421方法
  lowDec = (sub_low[0] - 48) * 8 + (sub_low[1] - 48) * 4 + (sub_low[2] - 48) * 2 + (sub_low[3] - 48);

  finalHex += hexMap[highDec];
  finalHex += hexMap[lowDec];
  finalHex.insert(0, "0X");
  stringstream ss;
  uint8_t x;
  unsigned int tmp;
  ss << finalHex;
  ss >> hex >> tmp;
  x = tmp;
  return x;
}

void readBinFiel(int width, unsigned char* imgWithHex, String bitStr) {
  String aLineBin;
  String sub;
  int pose = 0;                     //原始bit字符串截取位置
  int poseSub = 0;                  //每行bit串截取位置
  int bitLength = bitStr.length();  //图像像素比特串全长
  int times = width / 8;            //一行需要处理的次数
  int arrayPose = 0;
  unsigned int time = micros();
  for (int i = 0; i < 64; i++) {
    aLineBin = bitStr.substring(pose, pose + width);  //取一行
    pose += width;
    for (int n = 0; n < times; n++) {  //处理此行
      sub = aLineBin.substring(poseSub, poseSub + 8);
      imgWithHex[arrayPose] = binToHexDeal8Bit(sub);  // 得到8位字串转十六进制
      arrayPose++;
      poseSub += 8;  //8bit = 1byte
    }
    poseSub = 0;
  }
}