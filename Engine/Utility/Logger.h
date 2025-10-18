#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>

namespace Engine {

    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    class Logger {
    public:
        // Singleton access - prevents multiple instances!
        static Logger& Get() {
            static Logger instance;
            return instance;
        }

        // Delete copy constructor and assignment (prevent multiple instances)
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void SetLogLevel(LogLevel level);
        void EnableFileLogging(const std::string& filepath);

        template<typename... Args>
        void Log(LogLevel level, Args&&... args) {
            if (level < m_MinLevel) return;

            std::lock_guard<std::mutex> lock(m_Mutex);

            std::ostringstream oss;
            oss << "[" << GetLevelString(level) << "] ";
            (oss << ... << args);

            std::string message = oss.str();
            std::cout << message << std::endl;

            if (m_FileLoggingEnabled && m_FileStream.is_open()) {
                m_FileStream << message << std::endl;
                m_FileStream.flush();
            }
        }

        // Convenience methods
        template<typename... Args>
        void Trace(Args&&... args) { Log(LogLevel::Trace, std::forward<Args>(args)...); }

        template<typename... Args>
        void Debug(Args&&... args) { Log(LogLevel::Debug, std::forward<Args>(args)...); }

        template<typename... Args>
        void Info(Args&&... args) { Log(LogLevel::Info, std::forward<Args>(args)...); }

        template<typename... Args>
        void Warning(Args&&... args) { Log(LogLevel::Warning, std::forward<Args>(args)...); }

        template<typename... Args>
        void Error(Args&&... args) { Log(LogLevel::Error, std::forward<Args>(args)...); }

        template<typename... Args>
        void Critical(Args&&... args) { Log(LogLevel::Critical, std::forward<Args>(args)...); }

    private:
        Logger() = default;
        ~Logger() {
            if (m_FileStream.is_open()) {
                m_FileStream.close();
            }
        }

        const char* GetLevelString(LogLevel level);

        LogLevel m_MinLevel = LogLevel::Info;
        std::ofstream m_FileStream;
        bool m_FileLoggingEnabled = false;
        std::mutex m_Mutex;  // Thread-safe logging
    };

    // Convenience macros
#define LOG_TRACE(...) Engine::Logger::Get().Trace(__VA_ARGS__)
#define LOG_DEBUG(...) Engine::Logger::Get().Debug(__VA_ARGS__)
#define LOG_INFO(...) Engine::Logger::Get().Info(__VA_ARGS__)
#define LOG_WARNING(...) Engine::Logger::Get().Warning(__VA_ARGS__)
#define LOG_ERROR(...) Engine::Logger::Get().Error(__VA_ARGS__)
#define LOG_CRITICAL(...) Engine::Logger::Get().Critical(__VA_ARGS__)

} // namespace Engine