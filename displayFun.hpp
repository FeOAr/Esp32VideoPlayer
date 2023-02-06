#include "waitPicture.h"

extern int width;      //一行像素数（补齐到8的倍数）
extern int bitLength;  //图像像素比特串全长
extern U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2;

void drawImg(unsigned char *readyDrawHexArray, int fps) {
  u8g2.clearBuffer();
  /*--------------------*/
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(1);
  u8g2.drawXBMP(0, 0, width, 64, readyDrawHexArray);
  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.setCursor(102, 60);
  u8g2.print(fps);
  /*--------------------*/
  u8g2.sendBuffer();
}

void drawCover() {
  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  u8g2.setBitmapMode(1);
  u8g2.drawXBMP(-3, 6, 128, 51, coverTOHA);
  u8g2.sendBuffer();
}