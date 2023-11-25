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

IResult<SelectorDependency> combinator(absl::string_view input)
{
	auto [output1, value1] = *opt(spaces)(input);

	SelectorDependency dep;
	if (absl::StartsWith(output1, ">")) {
		dep = SelectorDependency::DirectParent;
	} else if (value1) {
		dep = SelectorDependency::Ancestor;
	} else {
		return absl::InvalidArgumentError("No combinator found.");
	}

	std::tie(output1, std::ignore) = *opt(spaces)(output1);

	return std::make_tuple(output1, dep);
}

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

IResult<std::vector<std::unique_ptr<Selector>>> selector_group(absl::string_view input)
{
	auto sel1_res = selector(input);
	if (!sel1_res.ok())
		return sel1_res.status();
	auto&& [output1, sel1] = *sel1_res;
	std::vector<std::unique_ptr<Selector>> sels;
	sels.emplace_back(std::move(sel1));

	while (true) {
		auto res2 = seq(seq(opt(spaces), tag(",")), selector)(output1);
		if (!res2.ok())
			break;

		auto&& [output2, tup] = *res2;
		auto&& [_, sel2] = tup;
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
IResult<Value> mess(absl::string_view input)
{
	return map(seq(number, mess_unit), [](auto&& t) {
		Value v;
		std::tie(v.f32_val, v.unit) = t;
		return v;
		})(input);
}

// Syntax: (IDENT | mess | STRING | hexcolor) S* 
IResult<SingleDeclaration> single_decl(base::string_atom name, absl::string_view input)
{
	IResult<SingleDeclaration> a;
	return a;
}

/* Syntax: mess S*
	| mess S+ mess S*
	| mess S+ mess S+ mess S*
	| mess S+ mess S+ mess S+ mess S*
 */
IResult<ShorthandDeclaration> magpad_shorthand_decl(base::string_atom name, absl::string_view input)
{
	IResult<ShorthandDeclaration> a;
	return a;
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
	if (prop_name == base::string_intern("margin")
		|| prop_name == base::string_intern("padding")) {
		decl = magpad_shorthand_decl(prop_name, input1);
	} else {
		decl = single_decl(prop_name, input1);
	}

	if (!decl.ok())
		return decl.status();


	return decl.value();
}

// Syntax: S* declaration (";" S* declaration)*
IResult<int> decls(absl::string_view input)
{
	auto res1 = tag("{")(input);
	if (!res1.ok())
		return res1.status();
	std::tie(input, std::ignore) = res1.value();

	std::tie(input, std::ignore) = opt(tag(";"))(input).value();
	std::tie(input, std::ignore) = opt(spaces)(input).value();

	auto res2 = tag("}")(input);
	if (!res2.ok())
		return res2.status();

	auto&& [output, _] = res2.value();
	return std::make_tuple(output, 0);
}

// Syntax: rule_rhs <-"{" decls ";"? S* "}" S*
IResult<StyleSpec> style_spec(absl::string_view input)
{
	auto res1 = tag("{")(input);
	if (!res1.ok())
		return res1.status();
	std::tie(input, std::ignore) = res1.value();

	// auto decls_res = 

	std::tie(input, std::ignore) = opt(tag(";"))(input).value();
	std::tie(input, std::ignore) = opt(spaces)(input).value();

	auto res2 = tag("}")(input);
	if (!res2.ok())
		return res2.status();

	std::tie(input, std::ignore) = opt(spaces)(input).value();
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
	// TODO: css rule specificity for pseudo-classes
	int s = 0;
	if (id != base::string_atom())
		s += 100;
	s += 10 * (int)klass.size();
	if (tag != base::string_atom())
		s += 1;
	if (dep_selector)
		s += dep_selector->specificity();
	return s;
}

}