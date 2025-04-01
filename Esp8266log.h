#ifndef ESP8266LOG_H
#define ESP8266LOG_H

#include <FS.h>
#include <vector>
#include <string>

struct LogEntry {
    std::string level;
    std::string tag;
    std::string message;
    unsigned long timestamp;
};

// 定义一个枚举类型，用于表示打印顺序
enum PrintOrder {
    ASCENDING,  // 正序
    DESCENDING // 倒序
};

class Logger {
public:
    Logger(const char* path);
    ~Logger();
    void log(const char* level, const char* tag, const char* message);
    void info(const char* tag, const char* message);
    void err(const char* tag, const char* message);
    void warn(const char* tag, const char* message);
    void debug(const char* tag, const char* message);
    void checkFlush();
    void flush();
    void printLog(PrintOrder order);
    size_t logCount();
    void clearFlashLogs();
    static void setFlashWriteInterval(unsigned long interval);
    static void setMaxLogEntries(size_t maxEntries);
private:
    static const char* logFilePath;
    static File logFile;
    static std::vector<LogEntry> logEntries;
    static const char* LOG_FILE_PATH;
    static unsigned long FLASH_WRITE_INTERVAL;
static size_t MAX_LOG_ENTRIES;
static unsigned long lastFlashWriteTime;
static unsigned long flashLogCount;
static std::string formatTime(unsigned long timestamp);
};

#endif