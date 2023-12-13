#include "ScriptValue.h"
#include "absl/types/variant.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"

namespace kwui {

class ScriptValue::Private {
public:
	struct Null {};

	absl::variant<Null, bool, double, std::string> var;
};

ScriptValue::ScriptValue()
	: d(new Private)
{}
ScriptValue::ScriptValue(bool v)
	: d(new Private)
{
	d->var.emplace<bool>(v);
}
ScriptValue::ScriptValue(int v)
	: d(new Private)
{
	d->var.emplace<double>((double)v);
}
ScriptValue::ScriptValue(double v)
	: d(new Private)
{
	d->var.emplace<double>(v);
}
ScriptValue::ScriptValue(const std::string& v)
	: d(new Private)
{
	d->var.emplace<std::string>(v);
}

ScriptValue::~ScriptValue()
{
	delete d;
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
	} else {
		return std::string();
	}
}


}