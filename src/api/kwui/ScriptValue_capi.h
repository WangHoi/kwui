#pragma once
#include "kwui_export.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct kwui_ScriptValue kwui_ScriptValue;
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptValue_newNull();
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptValue_newBool(bool val);
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptValue_newInt(int val);
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptValue_newDouble(double val);
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptValue_newString(const char* s, size_t len);
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptValue_newArray();
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptValue_newObject();
	KWUI_EXPORT void kwui_ScriptValue_delete(kwui_ScriptValue* v);

	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptValue_get_by_index(
		kwui_ScriptValue* arr, int index);
	KWUI_EXPORT void kwui_ScriptValue_set_by_index(
		kwui_ScriptValue* arr, int index, kwui_ScriptValue* val);
	KWUI_EXPORT kwui_ScriptValue* kwui_ScriptValue_get_by_str(
		kwui_ScriptValue* obj, const char* key);
	KWUI_EXPORT void kwui_ScriptValue_set_by_str(
		kwui_ScriptValue* obj, const char* key, kwui_ScriptValue* val);

	KWUI_EXPORT bool kwui_ScriptValue_is_null(kwui_ScriptValue* v);
	KWUI_EXPORT bool kwui_ScriptValue_is_bool(kwui_ScriptValue* v);
	KWUI_EXPORT bool kwui_ScriptValue_is_number(kwui_ScriptValue* v);
	KWUI_EXPORT bool kwui_ScriptValue_is_string(kwui_ScriptValue* v);
	KWUI_EXPORT bool kwui_ScriptValue_is_array(kwui_ScriptValue* v);
	KWUI_EXPORT bool kwui_ScriptValue_is_object(kwui_ScriptValue* v);
	
	KWUI_EXPORT bool kwui_ScriptValue_to_bool(kwui_ScriptValue* v);
	KWUI_EXPORT double kwui_ScriptValue_to_double(kwui_ScriptValue* v);
	KWUI_EXPORT int kwui_ScriptValue_to_int(kwui_ScriptValue* v);
	KWUI_EXPORT const char* kwui_ScriptValue_to_string(kwui_ScriptValue* v, size_t* len);

	KWUI_EXPORT size_t kwui_ScriptValue_length(kwui_ScriptValue* v);

	KWUI_EXPORT void kwui_ScriptValue_visitArray(
		kwui_ScriptValue* arr, void (*visitorFunction)(int index, const kwui_ScriptValue* val, void* udata), void* udata);
	KWUI_EXPORT void kwui_ScriptValue_visitObject(
		kwui_ScriptValue* obj, void (*visitorFunction)(const char* key, size_t key_len, const kwui_ScriptValue* val, void* udata), void* udata);

#ifdef __cplusplus
}
#endif

