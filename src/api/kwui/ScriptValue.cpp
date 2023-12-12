#include "ScriptValue.h"

namespace kwui {
ScriptValue::ScriptValue(void* ctx, uint64_t val)
	: ctx_(ctx), val_(val)
{
}
}