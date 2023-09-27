#include "EncodingManager.h"
#include "windows_header.h"

namespace windows {

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
std::wstring EncodingManager::UTF8ToWide(const std::string& text) {
	return CodePageToWide(text, CP_UTF8);
}
std::string EncodingManager::WideToUTF8(const std::wstring& text) {
	return WideToCodePage(text, CP_UTF8);
}

}
