#include "Logger.h"

namespace Engine {

    void Logger::SetLogLevel(LogLevel level) {
        m_MinLevel = level;
    }

    void Logger::EnableFileLogging(const std::string& filepath) {
        m_FileStream.open(filepath, std::ios::out | std::ios::trunc);
        m_FileLoggingEnabled = m_FileStream.is_open();

        if (m_FileLoggingEnabled) {
            LOG_INFO("File logging enabled: ", filepath);
        }
        else {
            LOG_ERROR("Failed to enable file logging: ", filepath);
        }
    }

    const char* Logger::GetLevelString(LogLevel level) {
        switch (level) {
        case LogLevel::Trace:    return "TRACE";
        case LogLevel::Debug:    return "DEBUG";
        case LogLevel::Info:     return "INFO";
        case LogLevel::Warning:  return "WARN";
        case LogLevel::Error:    return "ERROR";
        case LogLevel::Critical: return "CRITICAL";
        default:                 return "UNKNOWN";
        }
    }

} // namespace Engine