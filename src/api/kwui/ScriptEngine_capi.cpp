#include "ScriptEngine_capi.h"
#include "ScriptEngine.h"
#include "ScriptValue.h"
#include "base/log.h"
#include <set>
#include <vector>
#include <memory>

typedef std::tuple<std::string, kwui_ScriptFunction, void*> EventHandler;

static std::set<std::unique_ptr<EventHandler>> g_event_handlers;

static kwui::ScriptValue invoke_script_function(int argc, const kwui::ScriptValue* argv[], void* udata);
static EventHandler* find_handler(const char* event, kwui_ScriptFunction func, void* udata);
static EventHandler* find_handler(const char* event);
static EventHandler* add_handler(const char* event, kwui_ScriptFunction func, void* udata);
static std::unique_ptr<EventHandler> remove_handler(const char* event, kwui_ScriptFunction func, void* udata);
static std::unique_ptr<EventHandler> remove_handler(const char* event);

void kwui_ScriptEngine_addGlobalFunction(const char* name, kwui_ScriptFunction func, void* udata)
{
	if (find_handler(name))
		return;
	
	EventHandler* h = add_handler(name, func, udata);
	kwui::ScriptEngine::get()->addGlobalFunction(name, &invoke_script_function, h);
}

void kwui_ScriptEngine_removeGlobalFunction(const char* name)
{
	std::unique_ptr<EventHandler> h = remove_handler(name);
	kwui::ScriptEngine::get()->removeGlobalFunction(name);
}

void kwui_ScriptEngine_loadFile(const char* path)
{
	kwui::ScriptEngine::get()->loadFile(path);
}

kwui_ScriptValue* kwui_ScriptEngine_callGlobalFunction(const char* name, int argc, kwui_ScriptValue* argv[])
{
	std::vector<const kwui::ScriptValue*> args;
	for (int i = 0; i < argc; ++i) {
		args.push_back((const kwui::ScriptValue*)argv[i]);
	}
	kwui::ScriptValue ret = kwui::ScriptEngine::get()->callGlobalFunction(name, argc, args.data());
	return (kwui_ScriptValue*)new kwui::ScriptValue(std::move(ret));
}

void kwui_ScriptEngine_postEvent0(const char* event)
{
	//LOG(INFO) << "postEvent0 " << event;
	kwui::ScriptEngine::get()->postEvent(event);
}

void kwui_ScriptEngine_postEvent1(const char* event, kwui_ScriptValue* arg)
{
	//LOG(INFO) << "postEvent1 " << event;
	const kwui::ScriptValue& val = *(kwui::ScriptValue*)arg;
	kwui::ScriptEngine::get()->postEvent(event, val);
}

kwui_ScriptValue* kwui_ScriptEngine_sendEvent0(const char* event)
{
	kwui::ScriptValue ret = kwui::ScriptEngine::get()->sendEvent(event);
	return (kwui_ScriptValue*)new kwui::ScriptValue(std::move(ret));
}

kwui_ScriptValue* kwui_ScriptEngine_sendEvent1(const char* event, kwui_ScriptValue* arg)
{
	const kwui::ScriptValue& val = *(kwui::ScriptValue*)arg;
	kwui::ScriptValue ret = kwui::ScriptEngine::get()->sendEvent(event, val);
	return (kwui_ScriptValue*)new kwui::ScriptValue(std::move(ret));
}

kwui::ScriptValue invoke_script_function(int argc, const kwui::ScriptValue* argv[], void* udata)
{
	auto h = (EventHandler*)udata;
	kwui_ScriptFunction func = std::get<1>(*h);
	void* c_udata = std::get<2>(*h);
	std::vector<kwui_ScriptValue*> args;
	args.reserve(argc);
	for (int i = 0; i < argc; ++i) {
		args.push_back((kwui_ScriptValue*)argv[i]);
	}
	kwui_ScriptValue* c_ret = func(argc, args.data(), c_udata);
	kwui::ScriptValue ret(std::move(*(kwui::ScriptValue*)c_ret));
	kwui_ScriptValue_delete(c_ret);
	return ret;
}
EventHandler* find_handler(const char* event)
{
	for (auto& p : g_event_handlers) {
		if (std::get<0>(*p) == event) {
			return p.get();
		}
	}
	return nullptr;
}
EventHandler* find_handler(const char* event, kwui_ScriptFunction func, void* udata)
{
	for (auto& p : g_event_handlers) {
		if (std::get<0>(*p) == event
			&& std::get<1>(*p) == func
			&& std::get<2>(*p) == udata) {
			return p.get();
		}
	}
	return nullptr;
}
EventHandler* add_handler(const char* event, kwui_ScriptFunction func, void* udata)
{
	auto h = std::make_unique<EventHandler>(event, func, udata);
	auto ret = h.get();
	g_event_handlers.insert(std::move(h));
	return ret;
}
std::unique_ptr<EventHandler> remove_handler(const char* event, kwui_ScriptFunction func, void* udata)
{
	for (auto it = g_event_handlers.begin(); it != g_event_handlers.end(); ++it) {
		if (std::get<0>(**it) == event
			&& std::get<1>(**it) == func
			&& std::get<2>(**it) == udata) {
			return std::move(g_event_handlers.extract(it).value());
		}
	}
	return nullptr;
}
std::unique_ptr<EventHandler> remove_handler(const char* event)
{
	for (auto it = g_event_handlers.begin(); it != g_event_handlers.end(); ++it) {
		if (std::get<0>(**it) == event) {
			return std::move(g_event_handlers.extract(it).value());
		}
	}
	return nullptr;
}

void kwui_ScriptEngine_addEventListener(const char* event, kwui_ScriptFunction func, void* udata)
{
	LOG(INFO) << "addEventListener " << event;
	auto h = find_handler(event, func, udata);
	if (h) {
		LOG(ERROR) << "kwui_ScriptEngine_addEventListener: event " << event
			<< ", func " << func << ", udata " << udata << " already exists.";
		return;
	}
	h = add_handler(event, func, udata);
	kwui::ScriptEngine::get()->addEventListener(event, &invoke_script_function, h);
}

bool kwui_ScriptEngine_removeEventListener(const char* event, kwui_ScriptFunction func, void* udata)
{
	auto h = remove_handler(event, func, udata);
	if (!h) {
		LOG(ERROR) << "kwui_ScriptEngine_removeEventListener: event " << event
			<< ", func " << func << ", udata " << udata << " not exists.";
		return false;
	}
	return kwui::ScriptEngine::get()->removeEventListener(event, &invoke_script_function, h.get());
}
