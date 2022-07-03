#include "vfd.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESP32Time.h>
#include "sntp.h"

const char* ssid    = "WiFi名称";
const char* password = "WiFi密码";

const char* ntpServer1 = "cn.ntp.org.cn";   //中国
const char* ntpServer2 = "edu.ntp.org.cn";    //中国教育网
const long  gmtOffset_sec = 8 * 3600;   //参数就是用来修正时区的，比如对于我们东八区（UTC/GMT+08:00）来说该参数就需要填写 8 * 3600
const int   daylightOffset_sec = 0;     //使用夏令时 daylightOffset_sec 就填写3600，否则就填写0；

const char* time_zone = "CST-8";

WiFiMulti WiFiMulti;
ESP32Time rtc;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(100);

  vspi = new SPIClass(VSPI);
  vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_CS);

  pinMode(vspi->pinSS(), OUTPUT); //VSPI CS

  pinMode(RESET, OUTPUT);    //8bit硬重启
  digitalWrite(RESET, LOW);
  delay(50);
  digitalWrite(RESET, HIGH);

  delay(100);

  VFDinit();                //初始化显示，待机模式
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  if (WiFiMulti.run() == WL_CONNECTED) {
    VFDMode(false);        //运行模式
    VFDclearScreen();
    VFDWriteStr(6, "CONNECTED");    
  }

  sntp_servermode_dhcp(1);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    rtc.setTimeStruct(timeinfo);
  }
}

void loop()
{
  VFDclearScreen();
  VFDWriteStr(6, rtc.getDate());  
  VFDclearScreen();
  VFDWriteStr(6, rtc.getTime());  
}
