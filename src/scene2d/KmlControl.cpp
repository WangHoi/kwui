#include "KmlControl.h"
#include "Scene.h"
#include "api/kwui/ScriptEngine.h"

namespace scene2d {

const char* KmlControl::CONTROL_NAME = "kml";

KmlControl::KmlControl()
{}

base::string_atom KmlControl::name()
{
	return base::string_intern(CONTROL_NAME);
}
void KmlControl::onAnimationFrame(Node* node, absl::Time timestamp)
{
	const std::string id = node->scene()->eventContextId();
	kwui::ScriptValue arg = kwui::ScriptValue::newObject();
	arg["id"] = id;
	arg["timestamp"] = kwui::ScriptValue((double)absl::ToUnixMillis(timestamp));
	kwui::ScriptEngine::get()->postEvent("dialog:animation-event", arg);
	node->requestAnimationFrame(node);
}

}
