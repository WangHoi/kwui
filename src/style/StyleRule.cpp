#include "StyleRule.h"
#include "base/PEG.h"
#include <vector>

using namespace base::peg;

namespace style {

namespace {

struct SingleDeclaration {
	base::string_atom name;
	ValueSpec value;
};
struct ShorthandDeclaration {
	base::string_atom name;
	std::vector<SingleDeclaration> decls;
};

using Declaration = absl::variant<SingleDeclaration, ShorthandDeclaration>;

static StyleSpec make_style_spec(const std::vector<Declaration>& decls)
{
	StyleSpec spec;
	for (auto& decl : decls) {
		if (absl::holds_alternative<SingleDeclaration>(decl)) {
			const auto& sd = absl::get<SingleDeclaration>(decl);
			spec.set(sd.name, sd.value);
		} else if (absl::holds_alternative<ShorthandDeclaration>(decl)) {
			const auto& shd = absl::get<ShorthandDeclaration>(decl);
			for (const auto& sd : shd.decls) {
				spec.set(sd.name, sd.value);
			}
		}
	}
	return spec;
}

// Syntax: ("#" ident | "." ident | ":" ident)+
IResult<std::unique_ptr<Selector>> selector_item(absl::string_view input)
{
	auto [output1, _] = *opt(spaces)(input);

	Selector sel;
	auto id = seq(tag("#"), ident);
	auto klass = seq(tag("."), ident);
	auto psedudo_class = seq(tag(":"), ident);

	int count = 0;
	int n = 0;
	do {
		n = 0;
		{
			auto res = map(id, [&sel, &n](auto tup)
				{
					sel.id = base::string_intern(std::string(std::get<1>(tup)));
					return ++n;
				})(input);
				std::tie(input, std::ignore) = res.value_or(std::make_tuple(input, n));
		}
		{
			auto res = map(klass, [&sel, &n](auto tup) {
				sel.klass.addClass(base::string_intern(std::get<1>(tup)));
				return ++n;
				})(input);
				std::tie(input, std::ignore) = res.value_or(std::make_tuple(input, n));
		}
		{
			auto res = map(psedudo_class, [&sel, &n](auto tup) {
				sel.pseudo_classes.addClass(base::string_intern(std::get<1>(tup)));
				return ++n;
				})(input);
				std::tie(input, std::ignore) = res.value_or(std::make_tuple(input, n));
		}
		{
			auto res = map(ident, [&sel, &n](auto s) {
				sel.tag = base::string_intern(std::string(s));
				return ++n;
				})(input);
				std::tie(input, std::ignore) = res.value_or(std::make_tuple(input, n));
		}
		{
			auto res = map(tag("*"), [&sel, &n](auto s) {
				sel.tag = base::string_intern("*");
				return ++n;
				})(input);
				std::tie(input, std::ignore) = res.value_or(std::make_tuple(input, n));
		}
		count += n;
	} while (n > 0);

	if (count > 0)
		return std::make_tuple(input, std::make_unique<Selector>(std::move(sel)));
	else
		return absl::InvalidArgumentError("parse Selector error");
}

// Syntax: (" " | ">") S*
IResult<SelectorDependency> combinator(absl::string_view input)
{
	bool starts_with_space = absl::StartsWith(input, " ");
	absl::string_view output1;
	std::tie(output1, std::ignore) = *opt(spaces)(input);

	SelectorDependency dep;
	if (absl::StartsWith(output1, ">")) {
		output1 = output1.substr(1);
		dep = SelectorDependency::DirectParent;
	} else {
		if (starts_with_space) {
			dep = SelectorDependency::Ancestor;
		} else {
			return absl::InvalidArgumentError("No combinator found.");
		}
	}

	std::tie(output1, std::ignore) = *opt(spaces)(output1);

	return std::make_tuple(output1, dep);
}

// Syntax: selector_item (combinator selector_item)*
IResult<std::unique_ptr<Selector>> selector(absl::string_view input)
{
	auto sel1_res = selector_item(input);
	if (!sel1_res.ok())
		return sel1_res.status();
	auto&& [output1, sel1] = *sel1_res;
	std::unique_ptr<Selector> sel = std::move(sel1);

	auto res2 = seq(combinator, selector_item)(output1);
	while (res2.ok()) {
		auto&& [output2, tup] = *res2;
		auto&& [type, sel2] = tup;
		sel2->dep_type = type;
		sel2->dep_selector = std::move(sel);
		sel = std::move(sel2);

		output1 = output2;
		res2 = seq(combinator, selector_item)(output1);
	}
	return std::make_tuple(output1, std::move(sel));
}

// Syntax: selector ("," S* selector)*
IResult<std::vector<std::unique_ptr<Selector>>> selector_group(absl::string_view input)
{
	auto sel1_res = selector(input);
	if (!sel1_res.ok())
		return sel1_res.status();
	auto&& [output1, sel1] = *sel1_res;
	std::vector<std::unique_ptr<Selector>> sels;
	sels.emplace_back(std::move(sel1));

	while (true) {
		auto res2 = seq(tag(","), opt(spaces), selector)(output1);
		if (!res2.ok())
			break;

		auto&& [output2, tup] = *res2;
		auto&& [_1, _2, sel2] = tup;
		sels.emplace_back(std::move(sel2));

		output1 = output2;
	}
	return std::make_tuple(output1, std::move(sels));
}

// Syntax: prop_name <- IDENT S*
IResult<absl::string_view> prop_name(absl::string_view input)
{
	return map(seq(ident, opt(spaces)), [](auto&& t) {
		return absl::get<0>(t);
		})(input);
}

// Syntax: ("px" | "pt" | "%" | "")
IResult<ValueUnit> mess_unit(absl::string_view input)
{
	return alt(map(tag("px"), [](auto) { return ValueUnit::Pixel; }),
		map(tag("pt"), [](auto) { return ValueUnit::Point; }),
		map(tag("%"), [](auto) { return ValueUnit::Percent; }),
		map(tag(""), [](auto) { return ValueUnit::Raw; }))(input);
}

// Syntax: NUMBER mess_unit
IResult<Value> mess_value(absl::string_view input)
{
	return map(seq(number, mess_unit), [](auto&& t) {
		Value v;
		std::tie(v.f32_val, v.unit) = t;
		return v;
		})(input);
}

static inline bool is_hexchar(char ch)
{
	return (ch >= '0' && ch <= '9')
		|| (ch >= 'a' && ch <= 'f')
		|| (ch >= 'A' && ch <= 'F');
}

// Syntax: #RGB | #RGBA | #RRGGBB | #RRGGBBAA
IResult<Value> hexcolor_value(absl::string_view input)
{
	auto res = seq(tag("#"), take_while1(is_hexchar))(input);
	if (!res.ok())
		return res.status();
	auto&& [output, t] = res.value();
	auto&& [_, hex_str] = t;
	std::string hex_string(hex_str.data() - 1, hex_str.data() + hex_str.length());
	return std::make_tuple(output, Value::fromHexColor(hex_string));
}

// Syntax: "[^"]*"
IResult<Value> string_value(absl::string_view input)
{
	if (!absl::StartsWith(input, "\""))
		return absl::InvalidArgumentError(input);
	std::string s;
	size_t i;
	for (i = 1; i < input.length(); ++i) {
		if (input[i] == '\\') {
			if (i + 1 >= input.length())
				break;
			auto ch = input[++i];
			if (ch == 't') {
				s += '\t';
			} else if (ch == 'n') {
				s += '\n';
			} else if (ch == '\\') {
				s += '\\';
			} else if (ch == '"') {
				s += '"';
			} else {
				s += '\\';
				s += ch;
			}
		} else if (input[i] == '"') {
			break;
		} else {
			s += input[i];
		}
	}
	if (i >= input.length() || input[i] != '"')
		return absl::InvalidArgumentError(input);
	return std::make_tuple(input.substr(i + 1), Value::fromKeyword(base::string_intern(s)));
}

// Syntax: 'url("' [^"]* '")'
IResult<Value> url_value(absl::string_view input)
{
	if (!absl::StartsWith(input, "url(\""))
		return absl::InvalidArgumentError(input);
	std::string s;
	size_t i;
	for (i = 5; i < input.length(); ++i) {
		if (input[i] == '\\') {
			if (i + 1 >= input.length())
				break;
			auto ch = input[++i];
			if (ch == 't') {
				s += '\t';
			} else if (ch == 'n') {
				s += '\n';
			} else if (ch == '\\') {
				s += '\\';
			} else if (ch == '"') {
				s += '"';
			} else {
				s += '\\';
				s += ch;
			}
		} else if (input[i] == '"') {
			break;
		} else {
			s += input[i];
		}
	}
	if (i >= input.length() || !absl::StartsWith(input.substr(i), "\")"))
		return absl::InvalidArgumentError(input);
	return std::make_tuple(input.substr(i + 2), Value::fromUrl(s));
}

// Syntax: url | IDENT | mess | STRING | hexcolor
IResult<ValueSpec> value_spec(absl::string_view input)
{
	auto ident_res = ident(input);
	if (ident_res.ok()) {
		auto&& [output, atom_str] = ident_res.value();
		base::string_atom atom = base::string_intern(atom_str);
		ValueSpec spec;
		if (atom == base::string_intern("inherit")) {
			spec.type = ValueSpecType::Inherit;
			return std::make_tuple(output, spec);
		} else if (atom == base::string_intern("initial")) {
			spec.type = ValueSpecType::Inherit;
			return std::make_tuple(output, spec);
		}
	}

	auto ident_value = map(ident, [](auto s) { return Value::fromKeyword(base::string_intern(s)); });
	auto value = alt(url_value, ident_value, mess_value, string_value, hexcolor_value);
	return map(value, [&](auto val) {
		ValueSpec spec;
		spec.type = ValueSpecType::Specified;
		spec.value = val;
		return spec;
		})(input);
}

// Syntax: (IDENT | mess | STRING | hexcolor) S* 
IResult<SingleDeclaration> single_decl(base::string_atom name, absl::string_view input)
{
	auto res = value_spec(input);
	if (!res.ok())
		return res.status();
	auto&& [output, spec] = res.value();
	std::tie(output, std::ignore) = opt(spaces)(output).value();

	SingleDeclaration decl;
	decl.name = name;
	decl.value = spec;
	return std::make_tuple(output, decl);
}

/* Syntax: value_spec S*
	| value_spec S+ value_spec S*
	| value_spec S+ value_spec S+ value_spec S*
	| value_spec S+ value_spec S+ value_spec S+ value_spec S*
 */
IResult<ShorthandDeclaration> trbl_shorthand_decl(base::string_atom name,
	base::string_atom top_name,
	base::string_atom right_name,
	base::string_atom bottom_name,
	base::string_atom left_name,
	absl::string_view input)
{
	auto res = separated_list1(spaces, value_spec)(input);
	if (!res.ok())
		return res.status();
	auto&& [output, list] = res.value();
	ShorthandDeclaration sd;
	sd.name = name;
	
	if (list.size() == 1) {
		const ValueSpec& v0 = list[0];
		sd.decls.push_back(SingleDeclaration{ top_name, v0 });
		sd.decls.push_back(SingleDeclaration{ right_name, v0 });
		sd.decls.push_back(SingleDeclaration{ bottom_name, v0 });
		sd.decls.push_back(SingleDeclaration{ left_name, v0 });
	} else if (list.size() == 2) {
		const ValueSpec& v0 = list[0];
		const ValueSpec& v1 = list[1];
		sd.decls.push_back(SingleDeclaration{ top_name, v0 });
		sd.decls.push_back(SingleDeclaration{ right_name, v1 });
		sd.decls.push_back(SingleDeclaration{ bottom_name, v0 });
		sd.decls.push_back(SingleDeclaration{ left_name, v1 });
	} else if (list.size() == 3) {
		const ValueSpec& v0 = list[0];
		const ValueSpec& v1 = list[1];
		const ValueSpec& v2 = list[2];
		sd.decls.push_back(SingleDeclaration{ top_name, v0 });
		sd.decls.push_back(SingleDeclaration{ right_name, v1 });
		sd.decls.push_back(SingleDeclaration{ bottom_name, v2 });
		sd.decls.push_back(SingleDeclaration{ left_name, v1 });
	} else if (list.size() == 4) {
		const ValueSpec& v0 = list[0];
		const ValueSpec& v1 = list[1];
		const ValueSpec& v2 = list[2];
		const ValueSpec& v3 = list[3];
		sd.decls.push_back(SingleDeclaration{ top_name, v0 });
		sd.decls.push_back(SingleDeclaration{ right_name, v1 });
		sd.decls.push_back(SingleDeclaration{ bottom_name, v2 });
		sd.decls.push_back(SingleDeclaration{ left_name, v3 });
	}
	return std::make_tuple(output, sd);
}

/* Syntax: value_spec S*
	| value_spec S+ value_spec S*
 */
IResult<ShorthandDeclaration> two_value_shorthand_decl(base::string_atom name,
	base::string_atom one_name,
	base::string_atom two_name,
	absl::string_view input)
{
	auto res = separated_list1(spaces, value_spec)(input);
	if (!res.ok())
		return res.status();
	auto&& [output, list] = res.value();
	ShorthandDeclaration sd;
	sd.name = name;

	if (list.size() == 1) {
		const ValueSpec& v0 = list[0];
		sd.decls.push_back(SingleDeclaration{ one_name, v0 });
		sd.decls.push_back(SingleDeclaration{ two_name, v0 });
	} else if (list.size() == 2) {
		const ValueSpec& v0 = list[0];
		const ValueSpec& v1 = list[1];
		sd.decls.push_back(SingleDeclaration{ one_name, v0 });
		sd.decls.push_back(SingleDeclaration{ two_name, v1 });
	}
	return std::make_tuple(output, sd);
}

/* Syntax: value_spec S*
	| value_spec S+ value_spec S*
	| value_spec S+ value_spec S+ value_spec S*
 * Shorthand for: border-width || border-style || border-color
 */
IResult<ShorthandDeclaration> border_shorthand_decl(base::string_atom name, absl::string_view input)
{
	auto res = separated_list1(spaces, value_spec)(input);
	if (!res.ok())
		return res.status();
	auto&& [output, list] = res.value();
	ShorthandDeclaration sd;
	sd.name = name;

	absl::optional<ValueSpec> border_color;
	absl::optional<ValueSpec> border_width;
	for (const ValueSpec& spec : list) {
		if (spec.type != ValueSpecType::Specified)
			continue;
		const Value& val = spec.value.value();
		if (val.unit == ValueUnit::Keyword) {
			if (style::named_color::fromString(val.keyword_val.c_str()).has_value())
				border_color.emplace(spec);
		} else if (val.unit == ValueUnit::HexColor) {
			border_color.emplace(spec);
		} else if (val.unit == ValueUnit::Raw || val.unit == ValueUnit::Pixel
			|| val.unit == ValueUnit::Point || val.unit == ValueUnit::Percent) {
			border_width.emplace(spec);
		}
	}
	if (border_color.has_value()) {
		sd.decls.push_back(SingleDeclaration{ base::string_intern("border-color"), border_color.value()});
	}
	if (border_width.has_value()) {
		sd.decls.push_back(SingleDeclaration{ base::string_intern("border-left-width"), border_width.value() });
		sd.decls.push_back(SingleDeclaration{ base::string_intern("border-top-width"), border_width.value() });
		sd.decls.push_back(SingleDeclaration{ base::string_intern("border-right-width"), border_width.value() });
		sd.decls.push_back(SingleDeclaration{ base::string_intern("border-bottom-width"), border_width.value() });
	}
	return std::make_tuple(output, sd);
}

/* Syntax: value_spec S*
	| value_spec S+ value_spec S*
	| value_spec S+ value_spec S+ value_spec S*
 * Shorthand for: text-decoration-line || text-decoration-style || text-decoration-color
 */
IResult<ShorthandDeclaration> text_decoration_shorthand_decl(base::string_atom name, absl::string_view input)
{
	auto res = separated_list1(spaces, value_spec)(input);
	if (!res.ok())
		return res.status();
	auto&& [output, list] = res.value();
	ShorthandDeclaration sd;
	sd.name = name;

	absl::optional<ValueSpec> color;
	absl::optional<ValueSpec> line;
	for (const ValueSpec& spec : list) {
		if (spec.type != ValueSpecType::Specified)
			continue;
		const Value& val = spec.value.value();
		if (val.unit == ValueUnit::Keyword) {
			if (style::named_color::fromString(val.keyword_val.c_str()).has_value()) {
				color.emplace(spec);
			} else {
				if (val.keyword_val == base::string_intern("none")
					|| val.keyword_val == base::string_intern("underline")
					|| val.keyword_val == base::string_intern("overline")
					|| val.keyword_val == base::string_intern("line-through")) {
					line.emplace(spec);
				}
			}
		} else if (val.unit == ValueUnit::HexColor) {
			color.emplace(spec);
		}
	}
	if (color.has_value()) {
		sd.decls.push_back(SingleDeclaration{ base::string_intern("text-decoration-color"), color.value() });
	}
	if (line.has_value()) {
		sd.decls.push_back(SingleDeclaration{ base::string_intern("text-decoration-line"), line.value() });
	}
	return std::make_tuple(output, sd);
}

/* Syntax: value_spec S*
	| value_spec S+ value_spec S*
	| value_spec S+ value_spec S+ value_spec S*
	| value_spec S+ value_spec S+ value_spec S+ value_spec S*
 * Shorthand for: all
 *  | top-left-and-bottom-right top-right-and-bottom-left
 *  | top-left top-right-and-bottom-left bottom-right
 *  | top-left top-right bottom-right bottom-left
 */
IResult<ShorthandDeclaration> border_radius_shorthand_decl(base::string_atom name,
	base::string_atom top_left_name,
	base::string_atom top_right_name,
	base::string_atom bottom_right_name,
	base::string_atom bottom_left_name,
	absl::string_view input)
{
	auto res = separated_list1(spaces, value_spec)(input);
	if (!res.ok())
		return res.status();
	auto&& [output, list] = res.value();
	ShorthandDeclaration sd;
	sd.name = name;

	if (list.size() == 1) {
		const ValueSpec& v0 = list[0];
		sd.decls.push_back(SingleDeclaration{ top_left_name, v0 });
		sd.decls.push_back(SingleDeclaration{ top_right_name, v0 });
		sd.decls.push_back(SingleDeclaration{ bottom_right_name, v0 });
		sd.decls.push_back(SingleDeclaration{ bottom_left_name, v0 });
	} else if (list.size() == 2) {
		const ValueSpec& v0 = list[0];
		const ValueSpec& v1 = list[1];
		sd.decls.push_back(SingleDeclaration{ top_left_name, v0 });
		sd.decls.push_back(SingleDeclaration{ top_right_name, v1 });
		sd.decls.push_back(SingleDeclaration{ bottom_right_name, v0 });
		sd.decls.push_back(SingleDeclaration{ bottom_left_name, v1 });
	} else if (list.size() == 3) {
		const ValueSpec& v0 = list[0];
		const ValueSpec& v1 = list[1];
		const ValueSpec& v2 = list[2];
		sd.decls.push_back(SingleDeclaration{ top_left_name, v0 });
		sd.decls.push_back(SingleDeclaration{ top_right_name, v1 });
		sd.decls.push_back(SingleDeclaration{ bottom_right_name, v2 });
		sd.decls.push_back(SingleDeclaration{ bottom_left_name, v1 });
	} else if (list.size() == 4) {
		const ValueSpec& v0 = list[0];
		const ValueSpec& v1 = list[1];
		const ValueSpec& v2 = list[2];
		const ValueSpec& v3 = list[3];
		sd.decls.push_back(SingleDeclaration{ top_left_name, v0 });
		sd.decls.push_back(SingleDeclaration{ top_right_name, v1 });
		sd.decls.push_back(SingleDeclaration{ bottom_right_name, v2 });
		sd.decls.push_back(SingleDeclaration{ bottom_left_name, v3 });
	}
	return std::make_tuple(output, sd);
}


// Syntax: prop_name ":" S* prop_value prio? 
IResult<Declaration> declaration(absl::string_view input)
{
	auto prop_name_res = prop_name(input);
	if (!prop_name_res.ok())
		return prop_name_res.status();
	
	auto&& [input1, prop_name_str] = prop_name_res.value();
	auto res1 = seq(tag(":"), opt(spaces))(input1);
	if (!res1.ok())
		return res1.status();

	std::tie(input1, std::ignore) = res1.value();

	IResult<Declaration> decl;
	base::string_atom prop_name = base::string_intern(prop_name_str);
	if (prop_name == base::string_intern("margin")) {
		decl = trbl_shorthand_decl(prop_name,
			base::string_intern("margin-top"),
			base::string_intern("margin-right"),
			base::string_intern("margin-bottom"),
			base::string_intern("margin-left"),
			input1);
	} else if (prop_name == base::string_intern("padding")) {
		decl = trbl_shorthand_decl(prop_name,
			base::string_intern("padding-top"),
			base::string_intern("padding-right"),
			base::string_intern("padding-bottom"),
			base::string_intern("padding-left"),
			input1);
	} else if (prop_name == base::string_intern("border-width")) {
		decl = trbl_shorthand_decl(prop_name,
			base::string_intern("border-top-width"),
			base::string_intern("border-right-width"),
			base::string_intern("border-bottom-width"),
			base::string_intern("border-left-width"),
			input1);
	} else if (prop_name == base::string_intern("border-radius")) {
		decl = border_radius_shorthand_decl(prop_name,
			base::string_intern("border-top-left-radius"),
			base::string_intern("border-top-right-radius"),
			base::string_intern("border-bottom-right-radius"),
			base::string_intern("border-bottom-left-radius"),
			input1);
	} else if (prop_name == base::string_intern("inset")) {
		decl = trbl_shorthand_decl(prop_name,
			base::string_intern("top"),
			base::string_intern("right"),
			base::string_intern("bottom"),
			base::string_intern("left"),
			input1);
	} else if (prop_name == base::string_intern("overflow")) {
		decl = two_value_shorthand_decl(prop_name,
			base::string_intern("overflow-x"),
			base::string_intern("overflow-y"),
			input1);
	} else if (prop_name == base::string_intern("border")) {
		decl = border_shorthand_decl(prop_name, input1);
	} else if (prop_name == base::string_intern("text-decoration")) {
		decl = text_decoration_shorthand_decl(prop_name, input1);
	} else {
		decl = single_decl(prop_name, input1);
	}

	if (!decl.ok())
		return decl.status();

	// TODO: parse "prio?"

	return decl.value();
}

// Syntax: S* declaration (";" S* declaration)*
IResult<std::vector<Declaration>> decls(absl::string_view input)
{
	auto decl = map(seq(opt(spaces), declaration), [](auto&& t) { return absl::get<1>(t); });
	return separated_list1(tag(";"), decl)(input);
}

// rule_rhs <-"{" decls ";"? S* "}" S*
IResult<StyleSpec> rule_rhs(absl::string_view input)
{
	auto res1 = tag("{")(input);
	if (!res1.ok())
		return res1.status();
	std::tie(input, std::ignore) = res1.value();

	auto decls_res = decls(input);
	if (!decls_res.ok())
		return decls_res.status();

	std::vector<Declaration> decls;
	std::tie(input, decls) = decls_res.value();

	auto res2 = seq(opt(tag(";")), opt(spaces), tag("}"), opt(spaces))(input);
	if (!res2.ok())
		return res2.status();

	std::tie(input, std::ignore) = res2.value();

	StyleSpec spec = make_style_spec(decls);
	return std::make_tuple(input, spec);
}

// rule <- selector_group S* rule_rhs
IResult<std::vector<std::unique_ptr<StyleRule>>> rule(absl::string_view input)
{
	auto res = selector_group(input);
	if (!res.ok())
		return res.status();
	auto&& [output, sels] = *res;
	
	std::tie(output, std::ignore) = opt(spaces)(output).value();

	auto res1 = rule_rhs(output);
	if (!res1.ok())
		return res1.status();
	auto&& [output1, spec] = res1.value();

	std::vector<std::unique_ptr<StyleRule>> rules;
	for (auto it = sels.begin(); it != sels.end(); ++it) {
		auto r = std::make_unique<StyleRule>(std::move(*it), spec);
		rules.emplace_back(std::move(r));
	}
	return std::make_tuple(output1, std::move(rules));
}

} // namespace {}

absl::StatusOr<std::unique_ptr<Selector>> Selector::parse(absl::string_view input)
{
	auto res = selector(input);
	if (!res.ok())
		return res.status();
	auto&& [_, sel] = *res;
	return std::move(sel);
}

absl::StatusOr<std::vector<std::unique_ptr<Selector>>> Selector::parseGroup(absl::string_view input)
{
	auto res = selector_group(input);
	if (!res.ok())
		return res.status();
	auto&& [_, sels] = *res;
	return std::move(sels);
}

int Selector::specificity() const
{
	int s = 0;
	if (id != base::string_atom())
		s += 100;
	s += 10 * (int)klass.size();
	s += 10 * (int)pseudo_classes.size();
	if (tag != base::string_atom())
		s += 1;
	if (dep_selector)
		s += dep_selector->specificity();
	return s;
}

// S* rule*
absl::StatusOr<std::vector<std::unique_ptr<StyleRule>>> parse_css(absl::string_view input)
{
	std::tie(input, std::ignore) = opt(spaces)(input).value();

	std::vector<std::unique_ptr<StyleRule>> rules;
	auto res = rule(input);
	while (res.ok()) {
		auto&& [input1, sub_rules] = res.value();
		std::move(sub_rules.begin(), sub_rules.end(), std::back_inserter(rules));
		input = input1;
		res = rule(input);
	}
	if (input.length() > 0) {
		return absl::InvalidArgumentError(input);
	}
	return rules;
}

// decls S* ";"? S*
absl::StatusOr<StyleSpec> parse_inline_style(absl::string_view input)
{
	auto res = decls(input);
	if (!res.ok())
		return res.status();
	auto&& [input1, decls] = res.value();
	auto&& [output, _] = seq(opt(spaces), opt(tag(";")), opt(spaces))(input1).value();
	if (output.length() > 0) {
		return absl::InvalidArgumentError(output);
	}
	return make_style_spec(decls);
}

}