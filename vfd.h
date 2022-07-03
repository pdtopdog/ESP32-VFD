#include <SPI.h>

const int DisplayNumber  = 6;         // 显示字符总数
const int LeftScrollingDelayTime = 300;

const int VSPI_MISO  =  19 ;       // not connected
const int VSPI_MOSI   = 23 ;       // din
const int VSPI_SCLK   = 18;         // clk
const int VSPI_CS  =  5 ;         // cs
const int RESET   =   17;         // rest

static const int spiClk = 500000;  // 如ESP32 请加一些延时，VFD SPI时钟频率如手册描述 0.5MHZ

SPIClass * vspi = NULL;           //设置实体

void VFDbrightness(unsigned int level) {
  vspi->beginTransaction(SPISettings(spiClk, LSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_CS, LOW);
  vspi->transfer(0xe4);            //用于设置的第一个字节地址 DisplayNumber
  vspi->transfer(level);            //最亮为255
  digitalWrite(VSPI_CS, HIGH);
  vspi->endTransaction();
}
void VFDclearScreen(void) {
  vspi->beginTransaction(SPISettings(spiClk, LSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_CS, LOW); 
  for (int i = 0; i < DisplayNumber; i++) {
    vspi->transfer(0x01);       //0x01 0x20都是空格
  }
  vspi->transfer(0xe8);  // 刷新
  digitalWrite(VSPI_CS, HIGH);
  vspi->endTransaction();
}

void VFDshow(void) {
  vspi->beginTransaction(SPISettings(spiClk, LSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_CS, LOW);
  vspi->transfer(0xe8);   // 刷新
  digitalWrite(VSPI_CS, HIGH);
  vspi->endTransaction();
}
void VFDMode(bool on) {
  vspi->beginTransaction(SPISettings(spiClk, LSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_CS, LOW);
  if (on) {
    vspi->transfer(0xed);     // 待机模式
  }
  else {
    vspi->transfer(0xec);     // 运行模式
  }
  digitalWrite(VSPI_CS, HIGH);
  vspi->endTransaction();
}

void VFDWriteASCii(int x, unsigned char chr) {
  vspi->beginTransaction(SPISettings(spiClk, LSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_CS, LOW);
  vspi->transfer(0x20 + x);
  vspi->transfer(chr);
  digitalWrite(VSPI_CS, HIGH);
  vspi->endTransaction();
}

void VFDWriteStr(int x, String str) {
  //str = translateSpecialChars(str);
  if (x > 0) {
    for (int i = 0; i < x; i++) {
      str = " " + str;             // 填充空白
    }
  }
  unsigned int L = str.length();   //发送了多少字符?
  char Transient[L];                     // 定义字符的工作数组
  str.toCharArray(Transient , L + 1);     // 将字符串移动到字符数组中

  vspi->beginTransaction(SPISettings(spiClk, LSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_CS, LOW);
  vspi->transfer(0x20);                // 基址寄存器DCRAM 0H

  if (L > DisplayNumber) {
    for (int i = 0; i < DisplayNumber - x; i++) {
      vspi->transfer(Transient[i]);         // 将第一批填充到可见字符的末尾
    }
    digitalWrite(VSPI_CS, HIGH);
    for (int i = 0; i < L - DisplayNumber; i++) {
      digitalWrite(VSPI_CS, LOW);
      vspi->transfer(0x20);                // 基址寄存器DCRAM 0H
      for (int j = 0; j < DisplayNumber; j++) {
        vspi->transfer(Transient[j + i + 1]);   // 向左滚动
      }
      digitalWrite(VSPI_CS, HIGH);
      delay(LeftScrollingDelayTime);                 // 控制滚动延迟时间
    }
  }
  else {
    for (int i = 0; i < L; i++) {
      vspi->transfer(Transient[i]);
    }
    for (int i = 0; i < DisplayNumber - L - x; i++) {
      vspi->transfer(0x20);            // 用空格填充结尾
    }
  }
  digitalWrite(VSPI_CS, HIGH);
  vspi->endTransaction();
}

void VFDinit() {
  vspi->beginTransaction(SPISettings(spiClk, LSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_CS, LOW);
  vspi->transfer(0xe0);            //设置数字的第一个字节地址
  vspi->transfer(0x05);            //6数字等于值5(十六进制0x05)
  digitalWrite(VSPI_CS, HIGH);
  vspi->endTransaction();
 
  VFDbrightness(240);              //将亮度设置为初始值
  VFDshow();                       //激活屏幕
  for (int i = 0; i < 6; i++) {
    VFDWriteASCii(i, 0x7F);
  }
  delay(1000);
  VFDclearScreen();               //clear the screen
  VFDMode(true);
}
