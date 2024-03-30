#include "EncodingManager.h"
#ifdef _WIN32
#include "windows/windows_header.h"
#else
#include "utf.h"
#endif

namespace base {
#ifdef _WIN32
static std::string WideToCodePage(const std::wstring& text, UINT cp) {
	int utf8_size = ::WideCharToMultiByte(cp, 0,
		text.data(), (int)text.length(),
		NULL, 0, NULL, NULL);
	if (utf8_size <= 0)
		return std::string();
	std::string str;
	str.resize(utf8_size);
	::WideCharToMultiByte(cp, 0,
		text.data(), (int)text.length(),
		&str[0], str.length(), NULL, NULL);
	return str;
}
static std::wstring CodePageToWide(const std::string& text, UINT cp) {
	int wchar_size = ::MultiByteToWideChar(
		cp, 0, text.data(), (int)text.length(), NULL, 0);
	if (wchar_size <= 0)
		return std::wstring();
	std::wstring wstr;
	wstr.resize(wchar_size);
	::MultiByteToWideChar(cp,
		0,
		text.data(),
		(int)text.length(),
		&wstr[0],
		wchar_size);
	return wstr;
}
#endif
std::wstring EncodingManager::UTF8ToWide(const std::string& text) {
#ifdef _WIN32
	return CodePageToWide(text, CP_UTF8);
#else
	return utf::utf8_to_utf16<std::wstring>(text.begin(), text.end());
#endif
}
std::string EncodingManager::WideToUTF8(const std::wstring& text) {
#ifdef _WIN32
	return WideToCodePage(text, CP_UTF8);
#else
	static_assert(sizeof(wchar_t) == sizeof(uint32_t), "wchar_t size mismatch");
	return utf::utf32_to_utf8<std::string>((const uint32_t*)text.data(), (const uint32_t*)text.data() + text.size());
#endif
}

}
