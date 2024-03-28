#include "ScriptValue_capi.h"
#include "ScriptValue.h"
#include "absl/base/internal/per_thread_tls.h"
#include <stdlib.h>

kwui_ScriptValue* kwui_ScriptValue_newNull()
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue);
}

kwui_ScriptValue* kwui_ScriptValue_newBool(bool val)
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(val));
}

kwui_ScriptValue* kwui_ScriptValue_newInt(int val)
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(val));
}

kwui_ScriptValue* kwui_ScriptValue_newDouble(double val)
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(val));
}

kwui_ScriptValue* kwui_ScriptValue_newString(const char* s, size_t len)
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(std::string(s, len)));
}

kwui_ScriptValue* kwui_ScriptValue_newArray()
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(std::move(kwui::ScriptValue::newArray())));
}

kwui_ScriptValue* kwui_ScriptValue_newObject()
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(std::move(kwui::ScriptValue::newObject())));
}

void kwui_ScriptValue_delete(kwui_ScriptValue* v)
{
	delete (kwui::ScriptValue*)(v);
}

kwui_ScriptValue* kwui_ScriptValue_get_by_index(kwui_ScriptValue* arr, int index)
{
	kwui::ScriptValue& cpp_arr = *(kwui::ScriptValue*)arr;
	return (kwui_ScriptValue*)new kwui::ScriptValue(std::move(cpp_arr[index]));
}

void kwui_ScriptValue_set_by_index(kwui_ScriptValue* a, int index, kwui_ScriptValue* v)
{
	kwui::ScriptValue& arr = *(kwui::ScriptValue*)a;
	arr[index] = *(kwui::ScriptValue*)v;
}

kwui_ScriptValue* kwui_ScriptValue_get_by_str(kwui_ScriptValue* o, const char* key)
{
	kwui::ScriptValue& obj = *(kwui::ScriptValue*)o;
	return (kwui_ScriptValue*)new kwui::ScriptValue(std::move(obj[key]));
}

void kwui_ScriptValue_set_by_str(kwui_ScriptValue* o, const char* key, kwui_ScriptValue* v)
{
	kwui::ScriptValue& obj = *(kwui::ScriptValue*)o;
	obj[key] = *(kwui::ScriptValue*)v;
}

bool kwui_ScriptValue_is_null(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.isNull();
}
bool kwui_ScriptValue_is_bool(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.isBool();
}
bool kwui_ScriptValue_is_number(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.isNumber();
}
bool kwui_ScriptValue_is_string(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.isString();
}
bool kwui_ScriptValue_is_array(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.isArray();
}
bool kwui_ScriptValue_is_object(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.isObject();
}
bool kwui_ScriptValue_to_bool(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.toBool();
}
double kwui_ScriptValue_to_double(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.toDouble();
}
int kwui_ScriptValue_to_int(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.toInt();
}
#pragma message("TODO: inspect thread_local")
static thread_local/*ABSL_PER_THREAD_TLS_KEYWORD*/ std::string gt_cache;
const char* kwui_ScriptValue_to_string(kwui_ScriptValue* v, size_t* len)
{

	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	gt_cache.assign(val.toString());
	*len = gt_cache.length();
	return gt_cache.c_str();
}

size_t kwui_ScriptValue_length(kwui_ScriptValue* v)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	return val.length();
}

void kwui_ScriptValue_visitArray(
	kwui_ScriptValue* v,
	void (*visitorFunction)(int index, const kwui_ScriptValue* val, void* udata),
	void* udata
) {
	kwui::ScriptValue& arr = *(kwui::ScriptValue*)v;
	arr.visitArray([&](int index, const kwui::ScriptValue& v) {
		visitorFunction(index, (const kwui_ScriptValue*)&v, udata);
		});
}
void kwui_ScriptValue_visitObject(
	kwui_ScriptValue* v,
	void (*visitorFunction)(const char* key, size_t key_len, const kwui_ScriptValue* val, void* udata),
	void* udata
) {
	kwui::ScriptValue& obj = *(kwui::ScriptValue*)v;
	obj.visitObject([&](const std::string& key, const kwui::ScriptValue& v) {
		visitorFunction(key.c_str(), key.size(), (const kwui_ScriptValue*)&v, udata);
		});
}
