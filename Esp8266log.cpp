#include "Esp8266log.h"
#include <FS.h>
#include <vector>
#include <string>
#include <cstdlib>
// 初始化静态成员
const char* Logger::logFilePath = nullptr;
File Logger::logFile;
std::vector<struct LogEntry> Logger::logEntries;
const char* Logger::LOG_FILE_PATH = "/data/log.txt";
unsigned long Logger::lastFlashWriteTime = 0;

// 定义日志结构体
struct LogEntry {
    std::string level;
    std::string tag;
    std::string message;
    unsigned long timestamp; // 新增时间戳字段
};

Logger::Logger(const char* path) {
    // 将初始化逻辑移到构造函数内部
    logFilePath = path;
    logFile = SPIFFS.open(logFilePath, "a");
    if (!logFile) {
        Serial.Println("Logger init err, Failed to open log file");
        return;
    }

    if (logFile) {
        while (logFile.available()) {
            std::string line = logFile.readStringUntil('\n').c_str();
            Serial.printf("line:%s",line.c_str());
            size_t timestampEnd = line.find(" ");
            if (timestampEnd != std::string::npos) {
                unsigned long timestamp = std::stoul(line.substr(0, timestampEnd));
                size_t levelEnd = line.find(" ", timestampEnd + 1);
                if (levelEnd != std::string::npos) {
                    std::string level = line.substr(timestampEnd + 1, levelEnd - timestampEnd - 1);
                    size_t tagEnd = line.find(" ", levelEnd + 1);
                    if (tagEnd != std::string::npos) {
                        std::string tag = line.substr(levelEnd + 1, tagEnd - levelEnd - 1);
                        std::string message = line.substr(tagEnd + 1);
                        logEntries.push_back({level, tag, message, timestamp});
                    }
                }
            }
        }
        logFile.close();
    }
}

Logger::~Logger() {
    if (logFile) {
        logFile.close();
    }
}

void Logger::log(const char* level, const char* tag, const char* message) {
    std::string tagStr = tag;
    std::string messageStr = message;
    unsigned long timestamp = millis(); // 获取当前时间戳

    time_t t = timestamp / 1000; // 转换为秒
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d:%02d", hour(t), minute(t), second(t));
    Serial.printf("log: %s %s %s %s\n",timeStr, level, tag, message);

    // 检查最近一次的事件是否和当前事件相同
    if (!logEntries.empty() && logEntries.back().level == level && logEntries.back().tag == tagStr && logEntries.back().message == messageStr) {
        logEntries.pop_back(); // 移除最后一条记录
    } else if (logEntries.size() > MAX_LOG_ENTRIES) { // 确保日志记录不超过指定数目
        logEntries.erase(logEntries.begin()); // 移除最老的一条记录
    }

    // 添加新的日志记录
    logEntries.push_back({level, tagStr, messageStr, timestamp});
    checkFlush(); // 检查是否需要写入Flash
}

void Logger::info(const char* tag, const char* message) {
    log("INFO", tag, message);
}

void Logger::err(const char* tag, const char* message) {
    log("ERR", tag, message);
}

void Logger::warn(const char* tag, const char* message) {
    log("WARN", tag, message);
}

void Logger::debug(const char* tag, const char* message) {
    log("DEBUG", tag, message);
}

// 将缓存中的日志写入 Flash
void Logger::checkFlush() {
    unsigned long currentTime = millis();
    if (currentTime - lastFlashWriteTime >= FLASH_WRITE_INTERVAL) {
        flush();
    }
}

void Logger::flush() {
    logFile = SPIFFS.open(logFilePath, "w");
    if (logFile) {
        for (const auto& entry : logEntries) {
           // 格式化日志条目，格式为 "时间戳 日志级别 标签 消息"
           std::string logLine = std::to_string(entry.timestamp) + " " + entry.level + " " + entry.tag + " " + entry.message + "\n";
           // 将日志条目写入文件
           logFile.print(logLine.c_str());
        }
        logFile.close();
        // 更新最后写入时间
        lastFlashWriteTime = millis();
        // 清空日志条目缓存
        logEntries.clear();
    }
}