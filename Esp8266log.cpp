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
unsigned long Logger::flashLogCount = 0;
unsigned long Logger::FLASH_WRITE_INTERVAL = 1500;
size_t Logger::MAX_LOG_ENTRIES = 100;
Logger::Logger(const char* path) {
    // 将初始化逻辑移到构造函数内部
    logFilePath = path;
    logFile = SPIFFS.open(logFilePath, "r");
    if (!logFile) {
        Serial.println("Logger init err, Failed to open log file");
        return;
    }
    while (logFile.available()) {
        String line = logFile.readStringUntil('\n');
        if (line.length() > 0) {
            flashLogCount++;
        }
    }
    logFile.close();
}

Logger::~Logger() {
    if (logFile) {
        logFile.close();
    }
}

void Logger::setFlashWriteInterval(unsigned long interval) {
    FLASH_WRITE_INTERVAL = interval;
}

void Logger::setMaxLogEntries(size_t maxEntries) {
    MAX_LOG_ENTRIES = maxEntries;
}

void Logger::log(const char* level, const char* tag, const char* message) {
    std::string tagStr = tag;
    std::string messageStr = message;
    unsigned long timestamp = millis(); // 获取当前时间戳

    std::string timeStr = formatTime(timestamp);
    Serial.printf("%s %s %s %s\n",timeStr.c_str(), level, tag, message);

    // 检查最近一次的事件是否和当前事件相同
    if (!logEntries.empty()) {
        const LogEntry& lastEntry = logEntries.back();
        // 修改比较逻辑，确保比较的是字符串内容
        if (lastEntry.level == level && lastEntry.tag == tagStr && lastEntry.message == messageStr && lastEntry.timestamp == timestamp) {
            return; // 如果相同则不添加新记录
        }
    }
    if (logEntries.size() > MAX_LOG_ENTRIES) { // 确保日志记录不超过指定数目
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
    unsigned long elapsedTime = currentTime - lastFlashWriteTime;
    if (elapsedTime >= FLASH_WRITE_INTERVAL) {
        // Serial.println("录入数据");
        flush();
    }
    // Serial.printf("currentTime:%lu    lastFlashWriteTime:%lu    差值:%lu    FLASH_WRITE_INTERVAL:%lu\n",currentTime,lastFlashWriteTime,elapsedTime,FLASH_WRITE_INTERVAL);
}

std::string Logger::formatTime(unsigned long timestamp) {
    time_t rawtime = timestamp / 1000;
    struct tm * timeinfo;
    // 设置时区（这里假设为北京时间，东八区）
    setenv("TZ", "CST-8", 1);
    tzset();
    timeinfo = localtime(&rawtime);
    if (timeinfo == nullptr) {
        // 处理异常情况，返回一个默认值
        return "Invalid time";
    }
    char buffer[80];
    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

// 修改 flush 函数
void Logger::flush() {
    if (logEntries.empty()) {
        return; // 如果没有日志需要写入，则直接返回 
    }
    if (logEntries.size() + flashLogCount <= MAX_LOG_ENTRIES) {
        if (!logFile) {
            logFile = SPIFFS.open(logFilePath, "a");
        }
        if (logFile) {
            for (const auto& entry : logEntries) {
                logFile.printf("%lu,%s,%s,%s\n", entry.timestamp, entry.level.c_str(), entry.tag.c_str(), entry.message.c_str());
            }
            logFile.close();
            flashLogCount=flashLogCount+logEntries.size();
            lastFlashWriteTime = millis();
            logEntries.clear();
        }
        return;
    }

    // 读取 Flash 中的日志
    std::vector<LogEntry> flashLogEntries;
    if (SPIFFS.exists(logFilePath)) {
        File file = SPIFFS.open(logFilePath, "r");
        if (file) {
            while (file.available()) {
                String line = file.readStringUntil('\n');
                if (line.length() > 0) {
                    int commaPos1 = line.indexOf(',');
                    int commaPos2 = line.indexOf(',', commaPos1 + 1);
                    int commaPos3 = line.indexOf(',', commaPos2 + 1);
                    unsigned long timestamp = line.substring(0, commaPos1).toInt();
                    std::string level = line.substring(commaPos1 + 1, commaPos2).c_str();
                    std::string tag = line.substring(commaPos2 + 1, commaPos3).c_str();
                    std::string message = line.substring(commaPos3 + 1).c_str();
                    flashLogEntries.emplace_back(LogEntry{level, tag, message, timestamp});
                }
            }
            file.close();
        }
    }

    // 合并 logEntries 和 flashLogEntries
    flashLogEntries.insert(flashLogEntries.end(), logEntries.begin(), logEntries.end());

    // 确保 Flash 中的日志数量不超过 MAX_LOG_ENTRIES
    if (flashLogEntries.size() > MAX_LOG_ENTRIES) {
        size_t toRemove = flashLogEntries.size() - MAX_LOG_ENTRIES;
        flashLogEntries.erase(flashLogEntries.begin(), flashLogEntries.begin() + toRemove);
    }

    // 清空 logEntries
    logEntries.clear();

    // 将合并后的日志写回 Flash
    if (SPIFFS.exists(logFilePath)) {
        SPIFFS.remove(logFilePath);
    }
    logFile = SPIFFS.open(logFilePath, "a");
    if (logFile) {
        flashLogCount=0;
        for (const auto& entry : flashLogEntries) {
            flashLogCount++;
            // Serial.printf("录入数据：timestamp:%lu,level:%s,tag:%s,message:%s\n", entry.timestamp, entry.level.c_str(), entry.tag.c_str(), entry.message.c_str());
            logFile.printf("%lu,%s,%s,%s\n", entry.timestamp, entry.level.c_str(), entry.tag.c_str(), entry.message.c_str());
        }
        logFile.close();
    }
    lastFlashWriteTime = millis();
}
// 清理 Flash 中的日志
void Logger::clearFlashLogs() {
    if (SPIFFS.exists(logFilePath)) {
        if (SPIFFS.remove(logFilePath)) {
            Serial.println("Flash logs cleared successfully.");
        } else {
            Serial.println("Failed to clear flash logs.");
        }
    } else {
        Serial.println("Log file does not exist.");
    }
}

// 读取 flash 中的日志并按照指定顺序打印
void Logger::printLog(PrintOrder order) {
    logFile = SPIFFS.open(logFilePath, "r");
    if (!logFile) {
        Serial.println("Failed to open log file");
        return;
    }

    std::vector<std::string> logLines;
    // 读取所有日志行到 vector 中
    while (logFile.available()) {
        std::string line = logFile.readStringUntil('\n').c_str();
        logLines.push_back(line);
    }
    logFile.close();

    if (order == ASCENDING) {
        // 正序打印日志
        for (const auto& line : logLines) {
            Serial.println(line.c_str());
        }
    } else if (order == DESCENDING) {
        // 倒序打印日志
        for (auto it = logLines.rbegin(); it != logLines.rend(); ++it) {
            Serial.println(it->c_str());
        }
    }
}

size_t Logger::logCount()  {
    return logEntries.size();
}
