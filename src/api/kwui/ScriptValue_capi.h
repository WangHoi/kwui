#pragma once
#include "kwui_export.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct kwui_ScriptValue kwui_ScriptValue;
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newNull();
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newBool(bool val);
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newInt(int val);
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newDouble(double val);
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newString(const char* s, size_t len);
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newArray();
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_newObject();
	void KWUI_EXPORT kwui_ScriptValue_delete(kwui_ScriptValue* v);

	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_get_by_index(
		kwui_ScriptValue* arr, int index);
	void KWUI_EXPORT kwui_ScriptValue_set_by_index(
		kwui_ScriptValue* arr, int index, kwui_ScriptValue* val);
	kwui_ScriptValue* KWUI_EXPORT kwui_ScriptValue_get_by_str(
		kwui_ScriptValue* obj, const char* key);
	void KWUI_EXPORT kwui_ScriptValue_set_by_str(
		kwui_ScriptValue* obj, const char* key, kwui_ScriptValue* val);

	bool KWUI_EXPORT kwui_ScriptValue_is_null(kwui_ScriptValue* v);
	bool KWUI_EXPORT kwui_ScriptValue_is_bool(kwui_ScriptValue* v);
	bool KWUI_EXPORT kwui_ScriptValue_is_number(kwui_ScriptValue* v);
	bool KWUI_EXPORT kwui_ScriptValue_is_string(kwui_ScriptValue* v);
	bool KWUI_EXPORT kwui_ScriptValue_is_array(kwui_ScriptValue* v);
	bool KWUI_EXPORT kwui_ScriptValue_is_object(kwui_ScriptValue* v);
	
	bool KWUI_EXPORT kwui_ScriptValue_to_bool(kwui_ScriptValue* v);
	double KWUI_EXPORT kwui_ScriptValue_to_double(kwui_ScriptValue* v);
	int KWUI_EXPORT kwui_ScriptValue_to_int(kwui_ScriptValue* v);
	const char* KWUI_EXPORT kwui_ScriptValue_to_string(kwui_ScriptValue* v, size_t* len);

	size_t KWUI_EXPORT kwui_ScriptValue_length(kwui_ScriptValue* arr, char* s, size_t capacity);

	void KWUI_EXPORT kwui_ScriptValue_visitArray(
		kwui_ScriptValue* arr, void (*VisitorFunction)(int index, kwui_ScriptValue* val));
	void KWUI_EXPORT kwui_ScriptValue_visitObject(
		kwui_ScriptValue* obj, void (*VisitorFunction)(const char* key, size_t key_len, kwui_ScriptValue* val));

#ifdef __cplusplus
}
#endif

