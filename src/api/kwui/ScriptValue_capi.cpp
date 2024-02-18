#include "ScriptValue_capi.h"
#include "ScriptValue.h"

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newNull()
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue);
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newBool(bool val)
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(val));
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newInt(int val)
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(val));
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newDouble(double val)
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(val));
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newString(const char* s, size_t len)
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(std::string(s, len)));
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newArray()
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(std::move(kwui::ScriptValue::newArray())));
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newObject()
{
	return (kwui_ScriptValue*)(new kwui::ScriptValue(std::move(kwui::ScriptValue::newObject())));
}

void KWUI_EXPORT kwui_ScriptValue_delete(kwui_ScriptValue* v)
{
	delete (kwui::ScriptValue*)(v);
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_get_by_index(kwui_ScriptValue* arr, int index)
{
	kwui::ScriptValue& cpp_arr = *(kwui::ScriptValue*)arr;
	return (kwui_ScriptValue*)new kwui::ScriptValue(std::move(cpp_arr[index]));
}

void KWUI_EXPORT kwui_ScriptValue_set_by_index(kwui_ScriptValue* arr, int index, kwui_ScriptValue* val)
{
}

kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_get_by_str(kwui_ScriptValue* obj, const char* key)
{
	return nullptr;
}

void KWUI_EXPORT kwui_ScriptValue_set_by_str(kwui_ScriptValue* obj, const char* key, kwui_ScriptValue* val)
{
	return;
}


