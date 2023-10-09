#pragma once

#include <string_view>

namespace windows {
namespace graphics {

class Color {
public:
	Color() : _c{} {}
	explicit Color(float gray) { Set(gray, gray, gray); }
	explicit Color(int gray) { Set(gray, gray, gray); }
	Color(float r, float g, float b) { Set(r, g, b, 1.0f); }
	Color(float r, float g, float b, float a) { Set(r, g, b, a); }
	Color(int r, int g, int b, int a = 255) { Set(r, g, b, a); }
	static Color FromString(std::string_view str); // from web color name #FFDDEE or #FFDDEEFF
	uint32_t GetGdiRgb() const;

	/***** accessors *****/
#define FIELD_ACCESSOR(Field, field, index) \
    float Get##Field() const { return _c[index]; } \
    void Set##Field(float field) { _c[index] = field; } \
    void Set##Field(int field) { _c[index] = field / 255.0f; }

	FIELD_ACCESSOR(Red, r, 0)
		FIELD_ACCESSOR(Green, g, 1)
		FIELD_ACCESSOR(Blue, b, 2)
		FIELD_ACCESSOR(Alpha, a, 3)

#undef FIELD_ACCESSOR

		void Set(float r, float g, float b, float a = 1.0f) {
		_c[0] = r;
		_c[1] = g;
		_c[2] = b;
		_c[3] = a;
	}

	void Set(int r, int g, int b, int a = 255) {
		Set(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
	}

	const float* GetBuffer() const { return _c; }
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
	void UpperClamp() {
		for (int i = 0; i < 4; i++) {
			if (_c[i] > 1.0f) _c[i] = 1.0f;
		}
	}

	Color MakeInverse() const {
		return Color(1 - _c[0], 1 - _c[1], 1 - _c[2], _c[3]);
	}

	Color MakeMultiple(float k, bool keep_alpha) const {
		if (keep_alpha) return Color(_c[0] * k, _c[1] * k, _c[2] * k, _c[3]);
		else return Color(_c[0] * k, _c[1] * k, _c[2] * k, _c[3] * k);
	}

	Color MakeTransparent(float alpha) const {
		return Color(_c[0], _c[1], _c[2], _c[3] * alpha);
	}

	Color MakeAlpha(float alpha) const {
		return Color(_c[0], _c[1], _c[2], alpha);
	}

	// {sR, sG, sB, A} -> {sRsA, sGsA, sBsA, A}
	Color MakePremultipliedAlpha() const {
		float a = _c[3];
		return { _c[0] * a, _c[1] * a, _c[2] * a, a };
	}

	// {sR, sG, sB, A} -> {sRsA, sGsA, sBsA, A}
	Color MakePremultipliedAlpha(float more_alpha) const {
		float a = _c[3] * more_alpha;
		return{ _c[0] * a, _c[1] * a, _c[2] * a, a };
	}

	// {sR, sG, sB, A} -> {RA, GA, BA, A}
	Color MakePremultipliedLinearAlpha(float more_alpha) const {
		float a = _c[3] * more_alpha;
		return{ _c[0] * a, _c[1] * a, _c[2] * a, a };
	}

	// {sR, sG, sB, A} -> {R, G, B, A}
	Color MakeLinear() const {
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

#define NO_COLOR ::windows::graphics::Color{0, 0, 0, 0}
#define BLACK ::windows::graphics::Color{0, 0, 0}
#define WHITE ::windows::graphics::Color{255, 255, 255}
#define GRAY ::windows::graphics::Color{0.5f, 0.5f, 0.5f}
#define RED ::windows::graphics::Color{255, 0, 0}
#define GREEN ::windows::graphics::Color{0, 255, 0}
#define BLUE ::windows::graphics::Color{0, 0, 255}
#define YELLOW ::windows::graphics::Color{255, 255, 0}
#define MEGENTA ::windows::graphics::Color{255, 0, 255}
#define CYAN ::windows::graphics::Color{0, 255, 255}

} // namespace graphics
} // namespace windows
