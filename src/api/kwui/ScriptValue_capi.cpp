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

void kwui_ScriptValue_set_by_index(kwui_ScriptValue* arr, int index, kwui_ScriptValue* val)
{
}

kwui_ScriptValue* kwui_ScriptValue_get_by_str(kwui_ScriptValue* obj, const char* key)
{
	return nullptr;
}

void kwui_ScriptValue_set_by_str(kwui_ScriptValue* obj, const char* key, kwui_ScriptValue* val)
{
	return;
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
const char* kwui_ScriptValue_to_string(kwui_ScriptValue* v, size_t* len)
{
	kwui::ScriptValue& val = *(kwui::ScriptValue*)v;
	static ABSL_PER_THREAD_TLS_KEYWORD std::string s = val.toString();
	*len = s.length();
	return s.c_str();
}
