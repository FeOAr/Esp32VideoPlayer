#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include "bin2hex.hpp"
#include "uncompress.hpp"
#include "CircularQueue.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/rtc_wdt.h"
#include "displayFun.hpp"

U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/5, /* dc=*/16, /* reset=*/4);  //显示对象
const char *ssid = "K2P_4A_2.4GHZ";
const char *password = "19491001newChina";
const IPAddress serverIP(192, 168, 6, 105);  //目标地址
uint16_t serverPort = 6789;                  //目标服务器端口号
unsigned long int timeStep = 0;              //计时用时间辍
int width = 0;                               //一行像素数（补齐到8的倍数）
int bitLength;                               //图像像素比特串全长
int bitStingSize;                            //比特串字节数,即也是一张图的存储字节数
bool noPlaying = true;                       //封面和视频切换标志
xSemaphoreHandle cqMutex;                    //循环队列互斥锁
xSemaphoreHandle getFrameMutex;              //接收函数互斥访问
WiFiClient client;                           //声明一个客户端对象，用于与服务器进行连接
CirQueue CQobj;                              //循环队列对象
void TaskUncode_T1(void *pvParameters);      //处理线程
void TaskUncode_T2(void *pvParameters);
void TaskUncode_T3(void *pvParameters);
void TaskDisplay(void *pvParameters);  //显示线程
bool switchSave = true;                //两个解码线程互斥存储
long int frameID = 0;

void getWidth() {
  String temp;
  while (temp[0] == 0) {
    getFram(temp);
  }
  for (int i = 0; i < temp.length(); i++) {
    if ((temp[i] >= 48) && (temp[i] <= 57)) {
      width *= 10;
      width += (temp[i] - 48);
    }
  }
  if (width % 8 != 0) {
    width += (8 - width % 8);
  }
  bitLength = width * 64;  //初始化帧宽度，帧串长
  bitStingSize = (bitLength / 8);
}

void getFram(String &buf) {
  if (client.available() && client.connected()) {
    String line = client.readStringUntil('#');  //读取数据到换行符
    buf = line;
    if (buf.length() == 2 && buf == "**") {
      noPlaying = true;
    } else if (buf.length() > 1 && buf.length() < 4) {
      noPlaying = true;
    } else {
      noPlaying = false;
    }
  }
}

void fun_connect() {
  Serial.println("尝试访问SERVER");
  if (client.connect(serverIP, serverPort))  //尝试访问目标地址
  {
    Serial.println("访问成功");
    client.print("Hello world!");  //向服务器发送“hello world”
  } else {
    Serial.println("连接失败");
    client.stop();
  }
}

void setup(void) {
  u8g2.begin();
  Serial.begin(115200);
  rtc_wdt_protect_off();  //关闭看门狗写保护
  rtc_wdt_enable();
  WiFi.mode(WIFI_STA);   //站点模式
  WiFi.setSleep(false);  //关闭STA模式下wifi休眠，提高响应速度
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("已连接");
  Serial.print("本机ip地址:");
  Serial.println(WiFi.localIP());
  fun_connect();
  cqMutex = xSemaphoreCreateMutex();        //循环队列互斥锁
  getFrameMutex = xSemaphoreCreateMutex();  //循环队列互斥锁
  drawCover();
  getWidth();
  CQobj.queueInit();  //循环队列长度
  xTaskCreatePinnedToCore(TaskUncode_T1, "TaskUncode_T1", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TaskUncode_T2, "TaskUncode_T2", 4096, NULL, 2, NULL, tskNO_AFFINITY);
  xTaskCreatePinnedToCore(TaskUncode_T3, "TaskUncode_T3", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TaskDisplay, "Task_display", 2048, NULL, 1, NULL, tskNO_AFFINITY);
}

void loop(void) {
  rtc_wdt_feed();
}

void TaskUncode_T1(void *pvParameters) {  //处理线程
  (void)pvParameters;
  String cStrImg;          //压缩的bit串
  String unCStrImg;        //解压缩后
  bool nextPause = false;  //如果写满，暂时不写，等空位。
  bool framUpdate = true;  //最终十六进制数组帧更新标志，防止无意义重复写入循环队列
  unsigned char *testHexArray = (unsigned char *)malloc(sizeof(unsigned char) * bitStingSize);
  long int threadFrameID = 0;
  for (;;) {
    /*-----------解码 线程_1----------*/
    cStrImg = "";
    unCStrImg = "";
    if (!nextPause) {
      if (xSemaphoreTake(getFrameMutex, portMAX_DELAY)) {
        getFram(cStrImg);  //接收帧
        xSemaphoreGive(getFrameMutex);
      }
      if (cStrImg.length() > 10) {
        frameID++;
        threadFrameID = frameID;                      //成功读取到一帧
        decompression(cStrImg, unCStrImg);            //帧解压(String input, String &output)
        readBinFiel(width, testHexArray, unCStrImg);  //Bin转Hex
        framUpdate = false;  //当前帧未写入
      }
    }
    if (!framUpdate) {
      if (xSemaphoreTake(cqMutex, portMAX_DELAY)) {  //true 1线程可写
        if (CQobj.cqWrite(testHexArray, threadFrameID)) {
          CQobj.accum();
          nextPause = false;
          framUpdate = true;
        } else {
          nextPause = true;
          framUpdate = false;
        }
        xSemaphoreGive(cqMutex);
      }
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

int framNum = 0;
long int test = 0;

void TaskUncode_T2(void *pvParameters) {  //显示线程
  (void)pvParameters;
  String cStrImg_2;          //压缩的bit串
  String unCStrImg_2;        //解压缩后
  bool nextPause_2 = false;  //如果写满，暂时不写，等空位。
  bool framUpdate_2 = true;  //最终十六进制数组帧更新标志，防止无意义重复写入循环队列
  unsigned char *testHexArray_2 = (unsigned char *)malloc(sizeof(unsigned char) * bitStingSize);
  long int threadFrameID_2 = 0;
  for (;;) {
    rtc_wdt_feed();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    /*-----------解码 线程_2----------*/
    cStrImg_2 = "";
    unCStrImg_2 = "";
    if (!nextPause_2) {
      if (xSemaphoreTake(getFrameMutex, portMAX_DELAY)) {
        getFram(cStrImg_2);  //接收帧
        xSemaphoreGive(getFrameMutex);
      }
      if (cStrImg_2.length() > 10) {
        frameID++;
        threadFrameID_2 = frameID;                        //成功读取到一帧
        decompression(cStrImg_2, unCStrImg_2);            //帧解压
        readBinFiel(width, testHexArray_2, unCStrImg_2);  //Bin转Hex
        framUpdate_2 = false;                             //当前帧未写入
      }
    }
    if (!framUpdate_2) {
      if (xSemaphoreTake(cqMutex, portMAX_DELAY)) {
        if (CQobj.cqWrite(testHexArray_2, threadFrameID_2)) {
          CQobj.accum();
          nextPause_2 = false;
          framUpdate_2 = true;
        } else {
          nextPause_2 = true;
          framUpdate_2 = false;
        }
        xSemaphoreGive(cqMutex);
      }
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void TaskUncode_T3(void *pvParameters) {  //处理线程
  (void)pvParameters;
  String cStrImg;          //压缩的bit串
  String unCStrImg;        //解压缩后
  bool nextPause = false;  //如果写满，暂时不写，等空位。
  bool framUpdate = true;  //最终十六进制数组帧更新标志，防止无意义重复写入循环队列
  unsigned char *testHexArray = (unsigned char *)malloc(sizeof(unsigned char) * bitStingSize);
  long int threadFrameID = 0;
  for (;;) {
    /*-----------解码 线程_3----------*/
    cStrImg = "";
    unCStrImg = "";
    if (!nextPause) {
      if (xSemaphoreTake(getFrameMutex, portMAX_DELAY)) {
        getFram(cStrImg);  //接收帧
        xSemaphoreGive(getFrameMutex);
      }
      if (cStrImg.length() > 10) {
        frameID++;
        threadFrameID = frameID;                      //成功读取到一帧
        decompression(cStrImg, unCStrImg);            //帧解压
        readBinFiel(width, testHexArray, unCStrImg);  //Bin转Hex
        framUpdate = false;                           //当前帧未写入
      }
    }
    if (!framUpdate) {
      if (xSemaphoreTake(cqMutex, portMAX_DELAY)) {  //true 1线程可写
        if (CQobj.cqWrite(testHexArray, threadFrameID)) {
          CQobj.accum();
          nextPause = false;
          framUpdate = true;
        } else {
          nextPause = true;
          framUpdate = false;
        }
        xSemaphoreGive(cqMutex);
      }
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void TaskDisplay(void *pvParameters) {  //显示线程
  (void)pvParameters;
  int fps = 0;
  unsigned char *readyDrawHexArray = (unsigned char *)malloc(sizeof(unsigned char) * bitStingSize);
  for (;;) {
    rtc_wdt_feed();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    /*-----------显示部分----------*/
    if (CQobj.isEmpty()) {  //非空再申请锁
      if (xSemaphoreTake(cqMutex, portMAX_DELAY)) {
        CQobj.cqRead(readyDrawHexArray);
        xSemaphoreGive(cqMutex);
      }
    }
    if (noPlaying) {
      drawCover();
    } else {
      drawImg(readyDrawHexArray, fps);
    }
    if (millis() - timeStep >= 1000) {
      timeStep = millis();
      fps = framNum;
      framNum = 0;
    } else {
      framNum++;
    }
  }
}
