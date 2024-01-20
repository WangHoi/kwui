#pragma once
#include "quickjs.h"

namespace script {

class ContextId {
public:
	static JSClassID JS_CLASS_ID;
	static JSClassDef JS_CLASS_DEF;
	static void registerClass(JSContext* ctx);

	static JSValue create(JSContext* ctx);

	ContextId();
	~ContextId() = default;

	inline bool operator==(const ContextId& o) const {
		return id_ == o.id_;
	}
	inline bool operator==(size_t id) const {
		return id_ == id;
	}
	inline size_t id() const {
		return id_;
	}

private:
	static void finalize(JSRuntime* rt, JSValue val);

	size_t id_;
};

}