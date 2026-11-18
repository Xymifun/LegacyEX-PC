#ifndef PLATFORM_PC_SHADERPACK_LOG_HPP
#define PLATFORM_PC_SHADERPACK_LOG_HPP

// The logging calls of the shader pack code (adapted from LCEReborn), written to the game's log as
// "[pc] <name>: <message>" like the rest of the PC port.

#include <cstdarg>
#include <cstdio>

class Logger
{
public:
    explicit Logger(const char* name) : mName(name) {}

    const char* Name() const { return mName; }

    void Write(const char* level, const char* format, ...) const
    {
        std::printf("[pc] %s: %s", mName, level);
        va_list arguments;
        va_start(arguments, format);
        std::vprintf(format, arguments);
        va_end(arguments);
        std::printf("\n");
        std::fflush(stdout);
    }

private:
    const char* mName;
};

#define LOG_TRACE(logger, ...) ((void)0)
#define LOG_DEBUG(logger, ...) ((void)0)
#define LOG_INFO(logger, ...) (logger).Write("", __VA_ARGS__)
#define LOG_WARN(logger, ...) (logger).Write("warning: ", __VA_ARGS__)
#define LOG_ERROR(logger, ...) (logger).Write("error: ", __VA_ARGS__)
#define LOG_FATAL(logger, ...) (logger).Write("fatal: ", __VA_ARGS__)

#endif
