module;
#include <string>
export module scene2d:Stage;
import base;
import :Actor;

namespace scene2d {

export class Stage : public base::Object {
public:
	Actor* createTextActor(const std::u8string &text);
	Actor* createElementActor(base::string_atom tag);
};

}
