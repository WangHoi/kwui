#pragma once
#include "base/string_intern.h"
#include "absl/types/optional.h"
#include "absl/strings/string_view.h"

namespace style {

class Color {
public:
	Color() : _c{} {}
	explicit Color(float gray) { set(gray, gray, gray); }
	explicit Color(int gray) { set(gray, gray, gray); }
	Color(float r, float g, float b) { set(r, g, b, 1.0f); }
	Color(float r, float g, float b, float a) { set(r, g, b, a); }
	Color(int r, int g, int b, int a = 255) { set(r, g, b, a); }
	static Color fromString(std::string_view str); // from web color name #FFDDEE or #FFDDEEFF
	uint32_t getGdiRgb() const;

	/***** accessors *****/
#define FIELD_ACCESSOR(Field, field, index) \
    float get##Field() const { return _c[index]; } \
    void set##Field(float field) { _c[index] = field; } \
    void set##Field(int field) { _c[index] = field / 255.0f; }

	FIELD_ACCESSOR(Red, r, 0)
	FIELD_ACCESSOR(Green, g, 1)
	FIELD_ACCESSOR(Blue, b, 2)
	FIELD_ACCESSOR(Alpha, a, 3)

#undef FIELD_ACCESSOR

	void set(float r, float g, float b, float a = 1.0f) {
		_c[0] = r;
		_c[1] = g;
		_c[2] = b;
		_c[3] = a;
	}

	void set(int r, int g, int b, int a = 255) {
		set(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
	}

	const float* getBuffer() const { return _c; }
	float& operator[](int index) { return _c[index]; } // no bound check!
	const float& operator[](int index) const { return _c[index]; } // no bound check!
	Color& operator*=(const Color& c) {
		for (int i = 0; i < 4; ++i) _c[i] *= c._c[i];
		return *this;
	}
	Color& operator*=(float c) {
		for (int i = 0; i < 4; ++i) _c[i] *= c;
		return *this;
	}

	/***** operations *****/
	void upperClamp() {
		for (int i = 0; i < 4; i++) {
			if (_c[i] > 1.0f) _c[i] = 1.0f;
		}
	}

	Color makeInverse() const {
		return Color(1 - _c[0], 1 - _c[1], 1 - _c[2], _c[3]);
	}

	Color makeMultiple(float k, bool keep_alpha) const {
		if (keep_alpha) return Color(_c[0] * k, _c[1] * k, _c[2] * k, _c[3]);
		else return Color(_c[0] * k, _c[1] * k, _c[2] * k, _c[3] * k);
	}

	Color makeTransparent(float alpha) const {
		return Color(_c[0], _c[1], _c[2], _c[3] * alpha);
	}

	Color makeAlpha(float alpha) const {
		return Color(_c[0], _c[1], _c[2], alpha);
	}

	// {sR, sG, sB, A} -> {sRsA, sGsA, sBsA, A}
	Color makePremultipliedAlpha() const {
		float a = _c[3];
		return { _c[0] * a, _c[1] * a, _c[2] * a, a };
	}

	// {sR, sG, sB, A} -> {sRsA, sGsA, sBsA, A}
	Color makePremultipliedAlpha(float more_alpha) const {
		float a = _c[3] * more_alpha;
		return{ _c[0] * a, _c[1] * a, _c[2] * a, a };
	}

	// {sR, sG, sB, A} -> {RA, GA, BA, A}
	Color makePremultipliedLinearAlpha(float more_alpha) const {
		float a = _c[3] * more_alpha;
		return{ _c[0] * a, _c[1] * a, _c[2] * a, a };
	}

	// {sR, sG, sB, A} -> {R, G, B, A}
	Color makeLinear() const {
		return *this;
	}

private:
	float _c[4];
};

inline bool operator==(const Color& c1, const Color& c2) {
	for (int i = 0; i < 4; i++) {
		if (c1[i] != c2[i]) return false;
	}
	return true;
}

inline bool operator!=(const Color& c1, const Color& c2) {
	return !(c1 == c2);
}

namespace named_color {
absl::optional<const Color*> fromString(absl::string_view str);
extern const Color black;
extern const Color silver;
extern const Color gray;
extern const Color white;
extern const Color maroon;
extern const Color red;
extern const Color purple;
extern const Color fuchsia;
extern const Color green;
extern const Color lime;
extern const Color olive;
extern const Color yellow;
extern const Color navy;
extern const Color blue;
extern const Color teal;
extern const Color aqua;
extern const Color orange;
extern const Color aliceblue;
extern const Color antiquewhite;
extern const Color aquamarine;
extern const Color azure;
extern const Color beige;
extern const Color bisque;
extern const Color blanchedalmond;
extern const Color blueviolet;
extern const Color brown;
extern const Color burlywood;
extern const Color cadetblue;
extern const Color chartreuse;
extern const Color chocolate;
extern const Color coral;
extern const Color cornflowerblue;
extern const Color cornsilk;
extern const Color crimson;
extern const Color cyan;
extern const Color darkblue;
extern const Color darkcyan;
extern const Color darkgoldenrod;
extern const Color darkgray;
extern const Color darkgreen;
extern const Color darkgrey;
extern const Color darkkhaki;
extern const Color darkmagenta;
extern const Color darkolivegreen;
extern const Color darkorange;
extern const Color darkorchid;
extern const Color darkred;
extern const Color darksalmon;
extern const Color darkseagreen;
extern const Color darkslateblue;
extern const Color darkslategray;
extern const Color darkslategrey;
extern const Color darkturquoise;
extern const Color darkviolet;
extern const Color deeppink;
extern const Color deepskyblue;
extern const Color dimgray;
extern const Color dimgrey;
extern const Color dodgerblue;
extern const Color firebrick;
extern const Color floralwhite;
extern const Color forestgreen;
extern const Color gainsboro;
extern const Color ghostwhite;
extern const Color gold;
extern const Color goldenrod;
extern const Color greenyellow;
extern const Color grey;
extern const Color honeydew;
extern const Color hotpink;
extern const Color indianred;
extern const Color indigo;
extern const Color ivory;
extern const Color khaki;
extern const Color lavender;
extern const Color lavenderblush;
extern const Color lawngreen;
extern const Color lemonchiffon;
extern const Color lightblue;
extern const Color lightcoral;
extern const Color lightcyan;
extern const Color lightgoldenrodyellow;
extern const Color lightgray;
extern const Color lightgreen;
extern const Color lightgrey;
extern const Color lightpink;
extern const Color lightsalmon;
extern const Color lightseagreen;
extern const Color lightskyblue;
extern const Color lightslategray;
extern const Color lightslategrey;
extern const Color lightsteelblue;
extern const Color lightyellow;
extern const Color limegreen;
extern const Color linen;
extern const Color magenta;
extern const Color mediumaquamarine;
extern const Color mediumblue;
extern const Color mediumorchid;
extern const Color mediumpurple;
extern const Color mediumseagreen;
extern const Color mediumslateblue;
extern const Color mediumspringgreen;
extern const Color mediumturquoise;
extern const Color mediumvioletred;
extern const Color midnightblue;
extern const Color mintcream;
extern const Color mistyrose;
extern const Color moccasin;
extern const Color navajowhite;
extern const Color oldlace;
extern const Color olivedrab;
extern const Color orangered;
extern const Color orchid;
extern const Color palegoldenrod;
extern const Color palegreen;
extern const Color paleturquoise;
extern const Color palevioletred;
extern const Color papayawhip;
extern const Color peachpuff;
extern const Color peru;
extern const Color pink;
extern const Color plum;
extern const Color powderblue;
extern const Color rosybrown;
extern const Color royalblue;
extern const Color saddlebrown;
extern const Color salmon;
extern const Color sandybrown;
extern const Color seagreen;
extern const Color seashell;
extern const Color sienna;
extern const Color skyblue;
extern const Color slateblue;
extern const Color slategray;
extern const Color slategrey;
extern const Color snow;
extern const Color springgreen;
extern const Color steelblue;
extern const Color tan;
extern const Color thistle;
extern const Color tomato;
extern const Color turquoise;
extern const Color violet;
extern const Color wheat;
extern const Color whitesmoke;
extern const Color yellowgreen;
extern const Color rebeccapurple;
}

}
