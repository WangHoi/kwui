#include "StyleColor.h"
#include <unordered_map>

namespace style {

Color Color::fromString(std::string_view str) {
	if (str.empty()) return Color();
	if (str[0] == '#') {
		uint32_t c = (uint32_t)strtoul(&str[1], nullptr, 16);
		if (str.length() == 4) {
			int r = (c & 0xf00) >> 8;
			int g = (c & 0xf0) >> 4;
			int b = (c & 0xf);
			return Color((r << 4) | r, (g << 4) | g, (b << 4) | b);
		} else if (str.length() == 5) {
			int r = (c & 0xf000) >> 12;
			int g = (c & 0xf00) >> 8;
			int b = (c & 0xf0) >> 4;
			int a = (c & 0xf);
			return Color((r << 4) | r, (g << 4) | g, (b << 4) | b, (a << 4) | a);
		} else if (str.length() == 7) {
			return Color((int)((c & 0xff0000) >> 16), (int)((c & 0xff00) >> 8), (int)(c & 0xff));
		} else if (str.length() == 9) {
			return Color((int)((c & 0xff000000) >> 24), (int)((c & 0xff0000) >> 16), (int)((c & 0xff00) >> 8), (int)(c & 0xff));
		}
	} else {
		auto c = named_color::fromString(str);
		if (c.has_value())
			return *c.value();
	}
	return Color();
}

uint32_t Color::getGdiRgb() const {
	return (uint32_t(_c[0] * 255.0f) & 0xff)
		| ((uint32_t(_c[1] * 255.0f) & 0xff) << 8)
		| ((uint32_t(_c[2] * 255.0f) & 0xff) << 16);
}
#if WITH_SKIA
Color::operator SkColor4f() const
{
	return SkColor4f{ _c[0], _c[1], _c[2], _c[3] };
}
#endif
namespace named_color {
static std::unordered_map<std::string, const Color*> COLOR_TABLE = {
	{"black", &black},
	{"silver", &silver},
	{"gray", &gray},
	{"white", &white},
	{"maroon", &maroon},
	{"red", &red},
	{"purple", &purple},
	{"fuchsia", &fuchsia},
	{"green", &green},
	{"lime", &lime},
	{"olive", &olive},
	{"yellow", &yellow},
	{"navy", &navy},
	{"blue", &blue},
	{"teal", &teal},
	{"aqua", &aqua},
	{"orange", &orange},
	{"aliceblue", &aliceblue},
	{"antiquewhite", &antiquewhite},
	{"aquamarine", &aquamarine},
	{"azure", &azure},
	{"beige", &beige},
	{"bisque", &bisque},
	{"blanchedalmond", &blanchedalmond},
	{"blueviolet", &blueviolet},
	{"brown", &brown},
	{"burlywood", &burlywood},
	{"cadetblue", &cadetblue},
	{"chartreuse", &chartreuse},
	{"chocolate", &chocolate},
	{"coral", &coral},
	{"cornflowerblue", &cornflowerblue},
	{"cornsilk", &cornsilk},
	{"crimson", &crimson},
	{"cyan", &cyan},
	{"darkblue", &darkblue},
	{"darkcyan", &darkcyan},
	{"darkgoldenrod", &darkgoldenrod},
	{"darkgray", &darkgray},
	{"darkgreen", &darkgreen},
	{"darkgrey", &darkgrey},
	{"darkkhaki", &darkkhaki},
	{"darkmagenta", &darkmagenta},
	{"darkolivegreen", &darkolivegreen},
	{"darkorange", &darkorange},
	{"darkorchid", &darkorchid},
	{"darkred", &darkred},
	{"darksalmon", &darksalmon},
	{"darkseagreen", &darkseagreen},
	{"darkslateblue", &darkslateblue},
	{"darkslategray", &darkslategray},
	{"darkslategrey", &darkslategrey},
	{"darkturquoise", &darkturquoise},
	{"darkviolet", &darkviolet},
	{"deeppink", &deeppink},
	{"deepskyblue", &deepskyblue},
	{"dimgray", &dimgray},
	{"dimgrey", &dimgrey},
	{"dodgerblue", &dodgerblue},
	{"firebrick", &firebrick},
	{"floralwhite", &floralwhite},
	{"forestgreen", &forestgreen},
	{"gainsboro", &gainsboro},
	{"ghostwhite", &ghostwhite},
	{"gold", &gold},
	{"goldenrod", &goldenrod},
	{"greenyellow", &greenyellow},
	{"grey", &grey},
	{"honeydew", &honeydew},
	{"hotpink", &hotpink},
	{"indianred", &indianred},
	{"indigo", &indigo},
	{"ivory", &ivory},
	{"khaki", &khaki},
	{"lavender", &lavender},
	{"lavenderblush", &lavenderblush},
	{"lawngreen", &lawngreen},
	{"lemonchiffon", &lemonchiffon},
	{"lightblue", &lightblue},
	{"lightcoral", &lightcoral},
	{"lightcyan", &lightcyan},
	{"lightgoldenrodyellow", &lightgoldenrodyellow},
	{"lightgray", &lightgray},
	{"lightgreen", &lightgreen},
	{"lightgrey", &lightgrey},
	{"lightpink", &lightpink},
	{"lightsalmon", &lightsalmon},
	{"lightseagreen", &lightseagreen},
	{"lightskyblue", &lightskyblue},
	{"lightslategray", &lightslategray},
	{"lightslategrey", &lightslategrey},
	{"lightsteelblue", &lightsteelblue},
	{"lightyellow", &lightyellow},
	{"limegreen", &limegreen},
	{"linen", &linen},
	{"magenta", &magenta},
	{"mediumaquamarine", &mediumaquamarine},
	{"mediumblue", &mediumblue},
	{"mediumorchid", &mediumorchid},
	{"mediumpurple", &mediumpurple},
	{"mediumseagreen", &mediumseagreen},
	{"mediumslateblue", &mediumslateblue},
	{"mediumspringgreen", &mediumspringgreen},
	{"mediumturquoise", &mediumturquoise},
	{"mediumvioletred", &mediumvioletred},
	{"midnightblue", &midnightblue},
	{"mintcream", &mintcream},
	{"mistyrose", &mistyrose},
	{"moccasin", &moccasin},
	{"navajowhite", &navajowhite},
	{"oldlace", &oldlace},
	{"olivedrab", &olivedrab},
	{"orangered", &orangered},
	{"orchid", &orchid},
	{"palegoldenrod", &palegoldenrod},
	{"palegreen", &palegreen},
	{"paleturquoise", &paleturquoise},
	{"palevioletred", &palevioletred},
	{"papayawhip", &papayawhip},
	{"peachpuff", &peachpuff},
	{"peru", &peru},
	{"pink", &pink},
	{"plum", &plum},
	{"powderblue", &powderblue},
	{"rosybrown", &rosybrown},
	{"royalblue", &royalblue},
	{"saddlebrown", &saddlebrown},
	{"salmon", &salmon},
	{"sandybrown", &sandybrown},
	{"seagreen", &seagreen},
	{"seashell", &seashell},
	{"sienna", &sienna},
	{"skyblue", &skyblue},
	{"slateblue", &slateblue},
	{"slategray", &slategray},
	{"slategrey", &slategrey},
	{"snow", &snow},
	{"springgreen", &springgreen},
	{"steelblue", &steelblue},
	{"tan", &tan},
	{"thistle", &thistle},
	{"tomato", &tomato},
	{"turquoise", &turquoise},
	{"violet", &violet},
	{"wheat", &wheat},
	{"whitesmoke", &whitesmoke},
	{"yellowgreen", &yellowgreen},
	{"rebeccapurple", &rebeccapurple},
};
absl::optional<const Color*> fromString(absl::string_view str)
{
	auto it = COLOR_TABLE.find(std::string(str));
	return (it == COLOR_TABLE.end())
		? absl::nullopt
		: absl::make_optional(it->second);
}
const Color black(0, 0, 0);
const Color silver(192, 192, 192);
const Color gray(128, 128, 128);
const Color white(255, 255, 255);
const Color maroon(128, 0, 0);
const Color red(255, 0, 0);
const Color purple(128, 0, 128);
const Color fuchsia(255, 0, 255);
const Color green(0, 128, 0);
const Color lime(0, 255, 0);
const Color olive(128, 128, 0);
const Color yellow(255, 255, 0);
const Color navy(0, 0, 128);
const Color blue(0, 0, 255);
const Color teal(0, 128, 128);
const Color aqua(0, 255, 255);
const Color orange(255, 165, 0);
const Color aliceblue(240, 248, 255);
const Color antiquewhite(250, 235, 215);
const Color aquamarine(127, 255, 212);
const Color azure(240, 255, 255);
const Color beige(245, 245, 220);
const Color bisque(255, 228, 196);
const Color blanchedalmond(255, 235, 205);
const Color blueviolet(138, 43, 226);
const Color brown(165, 42, 42);
const Color burlywood(222, 184, 135);
const Color cadetblue(95, 158, 160);
const Color chartreuse(127, 255, 0);
const Color chocolate(210, 105, 30);
const Color coral(255, 127, 80);
const Color cornflowerblue(100, 149, 237);
const Color cornsilk(255, 248, 220);
const Color crimson(220, 20, 60);
const Color cyan(0, 255, 255);
const Color darkblue(0, 0, 139);
const Color darkcyan(0, 139, 139);
const Color darkgoldenrod(184, 134, 11);
const Color darkgray(169, 169, 169);
const Color darkgreen(0, 100, 0);
const Color darkgrey(169, 169, 169);
const Color darkkhaki(189, 183, 107);
const Color darkmagenta(139, 0, 139);
const Color darkolivegreen(85, 107, 47);
const Color darkorange(255, 140, 0);
const Color darkorchid(153, 50, 204);
const Color darkred(139, 0, 0);
const Color darksalmon(233, 150, 122);
const Color darkseagreen(143, 188, 143);
const Color darkslateblue(72, 61, 139);
const Color darkslategray(47, 79, 79);
const Color darkslategrey(47, 79, 79);
const Color darkturquoise(0, 206, 209);
const Color darkviolet(148, 0, 211);
const Color deeppink(255, 20, 147);
const Color deepskyblue(0, 191, 255);
const Color dimgray(105, 105, 105);
const Color dimgrey(105, 105, 105);
const Color dodgerblue(30, 144, 255);
const Color firebrick(178, 34, 34);
const Color floralwhite(255, 250, 240);
const Color forestgreen(34, 139, 34);
const Color gainsboro(220, 220, 220);
const Color ghostwhite(248, 248, 255);
const Color gold(255, 215, 0);
const Color goldenrod(218, 165, 32);
const Color greenyellow(173, 255, 47);
const Color grey(128, 128, 128);
const Color honeydew(240, 255, 240);
const Color hotpink(255, 105, 180);
const Color indianred(205, 92, 92);
const Color indigo(75, 0, 130);
const Color ivory(255, 255, 240);
const Color khaki(240, 230, 140);
const Color lavender(230, 230, 250);
const Color lavenderblush(255, 240, 245);
const Color lawngreen(124, 252, 0);
const Color lemonchiffon(255, 250, 205);
const Color lightblue(173, 216, 230);
const Color lightcoral(240, 128, 128);
const Color lightcyan(224, 255, 255);
const Color lightgoldenrodyellow(250, 250, 210);
const Color lightgray(211, 211, 211);
const Color lightgreen(144, 238, 144);
const Color lightgrey(211, 211, 211);
const Color lightpink(255, 182, 193);
const Color lightsalmon(255, 160, 122);
const Color lightseagreen(32, 178, 170);
const Color lightskyblue(135, 206, 250);
const Color lightslategray(119, 136, 153);
const Color lightslategrey(119, 136, 153);
const Color lightsteelblue(176, 196, 222);
const Color lightyellow(255, 255, 224);
const Color limegreen(50, 205, 50);
const Color linen(250, 240, 230);
const Color magenta(255, 0, 255);
const Color mediumaquamarine(102, 205, 170);
const Color mediumblue(0, 0, 205);
const Color mediumorchid(186, 85, 211);
const Color mediumpurple(147, 112, 219);
const Color mediumseagreen(60, 179, 113);
const Color mediumslateblue(123, 104, 238);
const Color mediumspringgreen(0, 250, 154);
const Color mediumturquoise(72, 209, 204);
const Color mediumvioletred(199, 21, 133);
const Color midnightblue(25, 25, 112);
const Color mintcream(245, 255, 250);
const Color mistyrose(255, 228, 225);
const Color moccasin(255, 228, 181);
const Color navajowhite(255, 222, 173);
const Color oldlace(253, 245, 230);
const Color olivedrab(107, 142, 35);
const Color orangered(255, 69, 0);
const Color orchid(218, 112, 214);
const Color palegoldenrod(238, 232, 170);
const Color palegreen(152, 251, 152);
const Color paleturquoise(175, 238, 238);
const Color palevioletred(219, 112, 147);
const Color papayawhip(255, 239, 213);
const Color peachpuff(255, 218, 185);
const Color peru(205, 133, 63);
const Color pink(255, 192, 203);
const Color plum(221, 160, 221);
const Color powderblue(176, 224, 230);
const Color rosybrown(188, 143, 143);
const Color royalblue(65, 105, 225);
const Color saddlebrown(139, 69, 19);
const Color salmon(250, 128, 114);
const Color sandybrown(244, 164, 96);
const Color seagreen(46, 139, 87);
const Color seashell(255, 245, 238);
const Color sienna(160, 82, 45);
const Color skyblue(135, 206, 235);
const Color slateblue(106, 90, 205);
const Color slategray(112, 128, 144);
const Color slategrey(112, 128, 144);
const Color snow(255, 250, 250);
const Color springgreen(0, 255, 127);
const Color steelblue(70, 130, 180);
const Color tan(210, 180, 140);
const Color thistle(216, 191, 216);
const Color tomato(255, 99, 71);
const Color turquoise(64, 224, 208);
const Color violet(238, 130, 238);
const Color wheat(245, 222, 179);
const Color whitesmoke(245, 245, 245);
const Color yellowgreen(154, 205, 50);
const Color rebeccapurple(102, 51, 153);
}

}