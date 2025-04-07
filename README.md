# ESP8266Log 日志库

一个简单高效的ESP8266日志记录库，支持内存缓存和SPIFFS闪存存储。

## 功能特性
- 精简化日志。相同连续内容只记录一次，减少日志文件大小；
- 多级别日志记录（DEBUG, INFO, WARN, ERROR）；
- 日志内存缓存，减少闪存写入次数；
- 自动将日志写入SPIFFS文件系统；
- 可配置的日志条目数量限制；
- 支持日志正序/倒序打印；
- 支持日志文件清理；


## 快速开始

### 安装
1. 将本库放入Arduino的libraries目录。一般位于 `C:\Users\你的用户名\Documents\Arduino\libraries`
2. 在项目中包含头文件：
```cpp
#include "Esp8266log.h"