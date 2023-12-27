#include "ScriptValue.h"
#include "absl/types/variant.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "fifo_map.hpp"
#include <vector>
using nlohmann::fifo_map;

namespace kwui {

static const ScriptValue g_null_value;
static ScriptValue g_null_ref;

class ScriptValue::Private {
public:
	struct Null {};

	absl::variant<Null, bool, double, std::string, std::vector<ScriptValue>, fifo_map<std::string, ScriptValue>> var;
};

ScriptValue::ScriptValue()
	: d(new Private)
{}
ScriptValue::ScriptValue(bool v)
	: ScriptValue()
{
	d->var.emplace<bool>(v);
}
ScriptValue::ScriptValue(int v)
	: ScriptValue()
{
	d->var.emplace<double>((double)v);
}
ScriptValue::ScriptValue(double v)
	: ScriptValue()
{
	d->var.emplace<double>(v);
}
ScriptValue::ScriptValue(const std::string& v)
	: ScriptValue()
{
	d->var.emplace<std::string>(v);
}

ScriptValue::ScriptValue(const ScriptValue& o)
	: ScriptValue()
{
	d->var = o.d->var;
}

ScriptValue::ScriptValue(ScriptValue&& o) noexcept
	: ScriptValue()
{
	d->var = std::move(o.d->var);
}

ScriptValue::~ScriptValue()
{
	delete d;
}

ScriptValue ScriptValue::newArray()
{
	ScriptValue val;
	val.d->var.emplace<std::vector<ScriptValue>>();
	return val;
}

ScriptValue ScriptValue::newObject()
{
	ScriptValue val;
	val.d->var.emplace<fifo_map<std::string, ScriptValue>>();
	return val;
}

ScriptValue& ScriptValue::operator=(const ScriptValue& o)
{
	d->var = o.d->var;
	return *this;
}

ScriptValue& ScriptValue::operator=(ScriptValue&& o) noexcept
{
	d->var = std::move(o.d->var);
	return *this;
}

const ScriptValue& ScriptValue::operator[](int idx) const
{
	if (!isArray())
		return g_null_value;
	const auto& arr = absl::get<std::vector<ScriptValue>>(d->var);
	if (idx < 0 || idx >= (int)arr.size())
		return g_null_value;
	return arr[idx];
}

ScriptValue& ScriptValue::operator[](int idx)
{
	if (!isArray() || idx < 0) {
		g_null_ref = ScriptValue();
		return g_null_ref;
	}
	auto& arr = absl::get<std::vector<ScriptValue>>(d->var);
	if (idx >= (int)arr.size()) {
		arr.resize(idx + 1);
	}
	return arr[idx];
}

const ScriptValue& ScriptValue::operator[](const char* key) const
{
	if (!isObject())
		return g_null_value;
	const auto& obj = absl::get<fifo_map<std::string, ScriptValue>>(d->var);
	auto it = obj.find(key);
	if (it == obj.end())
		return g_null_value;
	return it->second;
}

ScriptValue& ScriptValue::operator[](const char* key)
{
	if (!isObject()) {
		g_null_ref = ScriptValue();
		return g_null_ref;
	}
	auto& obj = absl::get<fifo_map<std::string, ScriptValue>>(d->var);
	auto it = obj.find(key);
	if (it == obj.end()) {
		std::string k(key);
		obj[k] = ScriptValue();
		return obj[k];
	}
	return it->second;
}

bool ScriptValue::isNull() const
{
	return absl::holds_alternative<Private::Null>(d->var);
}

bool ScriptValue::isBool() const
{
	return absl::holds_alternative<bool>(d->var);
}

bool ScriptValue::isNumber() const
{
	return absl::holds_alternative<double>(d->var);
}

bool ScriptValue::isString() const
{
	return absl::holds_alternative<std::string>(d->var);
}

bool ScriptValue::isArray() const
{
	return absl::holds_alternative<std::vector<ScriptValue>>(d->var);
}

bool ScriptValue::isObject() const
{
	return absl::holds_alternative<fifo_map<std::string, ScriptValue>>(d->var);
}

bool ScriptValue::toBool() const
{
	if (absl::holds_alternative<Private::Null>(d->var)) {
		return false;
	} else if (absl::holds_alternative<bool>(d->var)) {
		return absl::get<bool>(d->var);
	} else {
		return false;
	}
}

double ScriptValue::toDouble() const
{
	if (absl::holds_alternative<std::string>(d->var)) {
		double v = 0.0;
		absl::SimpleAtod(absl::get<std::string>(d->var), &v);
		return v;
	} else if (absl::holds_alternative<double>(d->var)) {
		return absl::get<double>(d->var);
	} else {
		return 0.0;
	}
}

int ScriptValue::toInt() const
{
	if (absl::holds_alternative<std::string>(d->var)) {
		int v = 0;
		absl::SimpleAtoi(absl::get<std::string>(d->var), &v);
		return v;
	} else if (absl::holds_alternative<double>(d->var)) {
		return (int)absl::get<double>(d->var);
	} else {
		return 0;
	}
}

std::string ScriptValue::toString() const
{
	if (absl::holds_alternative<std::string>(d->var)) {
		return absl::get<std::string>(d->var);
	} else if (absl::holds_alternative<bool>(d->var)) {
		return absl::get<bool>(d->var) ? "true" : "false";
	} else if (absl::holds_alternative<double>(d->var)) {
		return absl::StrCat(absl::get<double>(d->var));
	} else if (absl::holds_alternative<std::vector<ScriptValue>>(d->var)) {
		return "[array]";
	} else if (absl::holds_alternative<fifo_map<std::string, ScriptValue>>(d->var)) {
		return "[object]";
	} else {
		return std::string();
	}
}

void ScriptValue::visitArray(std::function<void(int, const ScriptValue&)>&& f) const
{
	if (!isArray())
		return;
	const auto& arr = absl::get<std::vector<ScriptValue>>(d->var);
	int i = 0;
	for (const auto& val : arr) {
		f(i++, val);
	}
}

void ScriptValue::visitObject(std::function<void(const std::string&, const ScriptValue&)>&& f) const
{
	if (!isObject())
		return;
	const auto& obj = absl::get<fifo_map<std::string, ScriptValue>>(d->var);
	for (const auto& p : obj) {
		f(p.first, p.second);
	}
}

}