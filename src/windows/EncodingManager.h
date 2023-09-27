#pragma once
#include <string>

namespace windows {

class EncodingManager {
public:
    static std::wstring UTF8ToWide(const std::string& text);
    static std::string WideToUTF8(const std::wstring& text);
};

}
