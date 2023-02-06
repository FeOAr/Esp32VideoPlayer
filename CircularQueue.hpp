#include "freertos/portmacro.h"
#include "freertos/FreeRTOS.h"

#define MAXSIZE 50  //队列总长度
extern int bitStingSize;


class CirQueue {
public:
  CirQueue();
  void queueInit();
  bool cqRead(unsigned char *);
  bool cqWrite(unsigned char *, long int);
  String getStatus();
  void print();
  bool isEmpty();
  void accum();

private:
  int front;  // 头指针，指向对头元素
  int rear;   // 尾指针，指向队尾元素的下一个位置，即最近空位
  int size;   // 队列已存储长度
  unsigned char *circularQueue[MAXSIZE];
  int ActualSize;           // 分配空间时按照字节计算，而不是自定义的单元
  long int saveFrameOrder;  //当前应该调用存储功能的线程序号
};

CirQueue::CirQueue() {
}

void CirQueue::queueInit() {
  ActualSize = sizeof(unsigned char) * bitStingSize;
  for (int i = 0; i < MAXSIZE; i++) {
    circularQueue[i] = (unsigned char *)malloc(ActualSize);
  }
  front = 0;
  rear = 0;
  size = 0;
  saveFrameOrder = 1;
}

bool CirQueue::cqRead(unsigned char *inPutArray) {
  if (size == 0) {
    return false;
  }
  for (int i = 0; i < bitStingSize; i++) {
    inPutArray[i] = circularQueue[front][i];
  }
  front = (front + 1) % MAXSIZE;
  size--;
  return true;
}

bool CirQueue::cqWrite(unsigned char *inPutArray, long int tFID) {
  if (size == MAXSIZE) {
    return false;
  } else if (tFID != saveFrameOrder) {
    return false;  //不是应该来存的线程
  }
  for (int i = 0; i < bitStingSize; i++) {
    circularQueue[rear][i] = inPutArray[i];
  }
  rear = (rear + 1) % MAXSIZE;
  size++;
  return true;
}

void CirQueue::print() {
  int n = 0;
  Serial.print("after write =>");
  for (; n < bitStingSize; n++) {
    Serial.print(circularQueue[0][n]);
    Serial.print(" ");
  }
  Serial.print("\n");
  // }
}

String CirQueue::getStatus() {
  String info = "front -> " + String(front) + "rear -> " + String(rear) + "size -> " + String(size);
  return info;
}

bool CirQueue::isEmpty() {
  if (size > 0) {
    return true;
  } else {
    return false;
  }
}

void CirQueue::accum() {
  saveFrameOrder++;
}