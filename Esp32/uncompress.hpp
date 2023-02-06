#include "HardwareSerial.h"



void decompression(String input, String &output) {
  int rept;
  for (int i = 0; i < input.length();) {
    rept = 0;
    int offset = 1;
    if (input[i] == 97 || input[i] == 98) {
      while (1) {
        if ((i + offset) < input.length() && input[i + offset] != 97 && input[i + offset] != 98) {
          rept *= 10;
          rept += (input[i + offset] - 48);  //在有跟数字情况下赋值
          offset++;
        } else {
          break;
        }
      }
    }
    char temp = input[i] - 49;
    output += temp;
    for (int n = 0; n < rept; n++) {
      output += temp;
    }
    i += offset;
  }
}