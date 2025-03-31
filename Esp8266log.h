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

private:
    static const char* logFilePath;
    static File logFile;
    static std::vector<LogEntry> logEntries;
    static const char* LOG_FILE_PATH;
    static const unsigned long FLASH_WRITE_INTERVAL = 1000;
static const size_t MAX_LOG_ENTRIES = 100;
static unsigned long lastFlashWriteTime;
};

#endif