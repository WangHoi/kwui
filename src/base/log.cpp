#include "log.h"
#include "absl/log/initialize.h"
#include "absl/log/log_sink_registry.h"
#ifdef _WIN32
#include "windows/windows_header.h"
#endif
#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace base {
#ifdef _WIN32
class DebugOutputLogSink : public absl::LogSink {
public:
    void Send(const absl::LogEntry& entry) override
    {
        const char* msg = entry.text_message_with_prefix_and_newline_c_str();
        OutputDebugStringA(msg);
    }
};
#endif
#ifdef __ANDROID__
class AndroidLogSink : public absl::LogSink {
public:
    void Send(const absl::LogEntry& entry) override
    {
        int prio;
        switch (entry.log_severity()) {
        case absl::LogSeverity::kInfo:
            prio = ANDROID_LOG_INFO;
            break;
        case absl::LogSeverity::kWarning:
            prio = ANDROID_LOG_WARN;
            break;
        case absl::LogSeverity::kError:
            prio = ANDROID_LOG_ERROR;
            break;
        case absl::LogSeverity::kFatal:
            prio = ANDROID_LOG_FATAL;
            break;
        default:
            prio = ANDROID_LOG_DEFAULT;
        }
        const char* msg = entry.text_message_with_prefix_and_newline_c_str();
        __android_log_write(prio, "kwui", msg);
    }
};
#endif
class StdErrLogSink : public absl::LogSink {
public:
    void Send(const absl::LogEntry& entry) override
    {
        const char* msg = entry.text_message_with_prefix_and_newline_c_str();
        fputs(msg, stderr);
    }
};

class CallbackLogSink : public absl::LogSink {
public:
    CallbackLogSink(kwui::LogCallback cb)
        : callback_(cb)
    {}
    void Send(const absl::LogEntry& entry) override
    {
        const char* msg = entry.text_message_with_prefix_and_newline_c_str();
        callback_(msg);
    }

private:
    kwui::LogCallback callback_;
};

void initialize_log(kwui::LogCallback func)
{
    absl::LogSink* sink = nullptr;
    if (func) {
        sink = new CallbackLogSink(func);
    } else {
#ifndef __ANDROID__
        sink = new StdErrLogSink;
#endif
    }
    if (sink) {
        absl::AddLogSink(sink);
    }
    absl::InitializeLog();
}

}

