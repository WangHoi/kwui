module;
#include <string>
#include <unordered_set>
export module base:string_intern;

namespace base {

export using string_atom = const char*;

export string_atom string_intern(const std::string& s)
{
	static std::unordered_set<std::string> tbl;
	auto p = tbl.insert(s);
	return p.first->c_str();
}

}
