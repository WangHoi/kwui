#include "log.h"
#include "absl/log/initialize.h"
#include "absl/log/log_sink_registry.h"
#include "windows/windows_header.h"

namespace base {

class DebugOutputLogSink : public absl::LogSink {
public:
    void Send(const absl::LogEntry& entry) override
    {
        const char* msg = entry.text_message_with_prefix_and_newline_c_str();
        OutputDebugStringA(msg);
    }
};

class StdErrLogSink : public absl::LogSink {
public:
    void Send(const absl::LogEntry& entry) override
    {
        const char* msg = entry.text_message_with_prefix_and_newline_c_str();
        fputs(msg, stderr);
    }
};

void initialize_log()
{
    absl::AddLogSink(new StdErrLogSink);
    absl::InitializeLog();
}

}

