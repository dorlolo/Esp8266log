#include <string.h>
#include <cstdlib>
#include <FS.h>
#include <Esp8266log.h>
#include <Arduino.h>
//---------------日志Tag
const char* TAG_NEWWORK = "NETWORK";
const char* TAG_OTA = "OTA";
const char* TAG_LED = "LED"; 
const char* TAG_USER = "USER";
const char* TAG_UTIL = "UTIL";
static unsigned long logIndex = 0;

Logger logger("/data/log.log");
void setup() {
  Serial.begin(115200);
  delay(1000);
  while(!Serial);
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  logger.setFlashWriteInterval(5000);
  logger.setMaxLogEntries(20);
  Serial.println("flash log list:");
  logger.printLog(ASCENDING);
  Serial.println("----------------------------------------");
  logger.clearFlashLogs();
}

void loop() {
  delay(1000);
  Serial.println("loop");
  recordLogDemo();
}

void recordLogDemo(){
  Serial.println(logger.logCount());
  char logMessage[64];
  snprintf(logMessage, sizeof(logMessage), "THIS IS A INFO LOG %lu", logIndex);
  logger.info(TAG_UTIL,logMessage);
  logIndex++;
}