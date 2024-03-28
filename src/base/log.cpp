#include "log.h"
#include "absl/log/initialize.h"
#include "absl/log/log_sink_registry.h"
#ifdef _WIN32
#include "windows/windows_header.h"
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
    absl::LogSink* sink;
    if (func) {
        sink = new CallbackLogSink(func);
    } else {
        sink = new StdErrLogSink;
    }
    absl::AddLogSink(sink);
    absl::InitializeLog();
}

}

