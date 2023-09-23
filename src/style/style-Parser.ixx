module;
#include <tao/pegtl.hpp>
#include <string>
#include <fifo_map.hpp>

export module style:Parser;

import base;
import :Style;

namespace style {

using nlohmann::fifo_map;
namespace pg = tao::pegtl;

class StylePropertyMap;

namespace parser {

struct S;
struct ws_or_nl;
struct comment;
struct more_declaration;
struct declaration;
struct property;
struct expr;
struct prio;
struct term;
struct operator_;
struct operator_a;
struct unary_op;
struct measure;
struct mess;
struct em_m;
struct ex_m;
struct px_m;
struct cm_m;
struct mm_m;
struct in_m;
struct pt_m;
struct pc_m;
struct deg_m;
struct rad_m;
struct grad_m;
struct ms_m;
struct s_m;
struct hx_m;
struct khz_m;
struct percent_m;
struct dim_m;
struct raw_m;
struct other_term;
struct function;
struct STRING;
struct URI;
struct hexcolor;

template <typename Rule> struct action : pg::nothing<Rule> {};

/*
ws <- [ \t]
nl <- [\n]
*/
// comment < -"/*" (!"*/" .)* "*/"
struct comment : pg::seq<TAO_PEGTL_STRING("/*"), pg::until<TAO_PEGTL_STRING("*/")>, TAO_PEGTL_STRING("*/")> {};

// S <- ws_or_nl/comment
struct S : pg::sor<pg::space, comment> {};

struct S_star : public pg::star<S> {};

struct IDENT : public pg::identifier {};

// NUMBER <- digit+ 
struct NUMBER : public pg::plus<pg::digit> {};

// decls <- S* declaration more_declaration*
struct decls : public pg::seq<S_star, pg::star<more_declaration>> {};

// more_declaration <- ";" S* declaration
struct more_declaration : public pg::seq<TAO_PEGTL_STRING(";"), S_star, declaration> {};

// declaration <- property ":" S* expr prio?
struct declaration : public pg::seq<property, TAO_PEGTL_STRING(":"), S_star, expr, pg::opt<prio>> {};

// property <- IDENT S*
struct property : public pg::seq<IDENT, S_star> {};

// prio < -"!" S* "important"
struct prio : public pg::seq<TAO_PEGTL_STRING("!"), S_star, TAO_PEGTL_STRING("important")> {};

// expr <- term ( operator_? term )*
struct expr : public pg::seq<term, pg::star<pg::seq<pg::star<operator_>, term>>> {};

// operator_ <- operator_a S*
struct operator_ : public pg::seq<operator_a, S_star> {};

// operator_a <-  "/" / ","
struct operator_a : public pg::sor<TAO_PEGTL_STRING("/"), TAO_PEGTL_STRING(",")> {};

// term <- unary_op measure / measure / other_term
struct term : public pg::sor<pg::seq<unary_op, measure>, measure, other_term> {};

// unary_op <- "-" / "+"
struct unary_op : public pg::sor<TAO_PEGTL_STRING("-"), TAO_PEGTL_STRING("+")> {};

// measure <- mess S*
struct measure : public pg::seq<mess, S_star> {};

// mess <- em_m / ex_m / px_m / cm_m / mm_m / in_m / pt_m / pc_m / deg_m / rad_m / grad_m / ms_m / s_m / hx_m / khz_m / percent_m / dim_m / raw_m
struct mess : public pg::sor<em_m, ex_m, px_m, cm_m, mm_m, in_m, pt_m, pc_m, deg_m, rad_m, grad_m, ms_m, s_m, hx_m, khz_m, percent_m, dim_m, raw_m> {};

// em_m <- NUMBER "em"
struct em_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("em")> {};

// ex_m <- NUMBER "ex"
struct ex_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("ex")> {};

// px_m <- NUMBER "px"
struct px_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("px")> {};

// cm_m <- NUMBER "cm"
struct cm_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("cm")> {};

// mm_m <- NUMBER "mm"
struct mm_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("mm")> {};

// in_m <- NUMBER "in"
struct in_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("in")> {};

// pt_m <- NUMBER "pt"
struct pt_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("pt")> {};

// pc_m <- NUMBER "pc"
struct pc_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("pc")> {};

// deg_m <- NUMBER "deg"
struct deg_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("deg")> {};

// rad_m <- NUMBER "rad"
struct rad_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("rad")> {};

// grad_m <- NUMBER "grad"
struct grad_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("grad")> {};

// ms_m <- NUMBER "ms"
struct ms_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("ms")> {};

// s_m <- NUMBER "s"
struct s_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("s")> {};

// hx_m <- NUMBER "hx"
struct hx_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("hx")> {};

// khz_m <- NUMBER "khz"
struct khz_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("khz")> {};

// percent_m <- NUMBER "%"
struct percent_m : public pg::seq<NUMBER, TAO_PEGTL_STRING("%")> {};

// dim_m <- NUMBER IDENT
struct dim_m : public pg::seq<NUMBER, IDENT> {};

// raw_m <- NUMBER
struct raw_m : public NUMBER {};

// other_term <- STRING S* / URI S* / IDENT S* / hexcolor S* / function
struct other_term : public pg::sor<
	pg::seq<STRING, S_star>,
	pg::seq<URI, S_star>,
	pg::seq<IDENT, S_star>,
	pg::seq<hexcolor, S_star>,
	function> {};

// function <- IDENT "(" S* expr ")" S*
struct function : public pg::seq<IDENT, TAO_PEGTL_STRING("("), S_star, expr, TAO_PEGTL_STRING(")"), S_star> {};

struct escaped_char : pg::seq<pg::one<'\\'>, pg::one<'"', '\'', '\\', '/', 'b', 'f', 'n', 'r', 't', 'v', '0'>> {};

// STRING <- ["] ( !["] . )* ["]
struct STRING : public pg::seq<pg::one<'"'>, pg::star<pg::sor<escaped_char, pg::not_one<'"', '\\'>>>, pg::one<'"'>> {};

// URI <- "url(" ( !")" . )* ")"
struct URI : public pg::seq<TAO_PEGTL_STRING("url("), pg::until<pg::one<')'>>, pg::one<')'>> {};

// hexcolor <- "#" hex hex hex hex hex hex / "#" hex hex hex
struct hexcolor : public pg::sor<
	pg::seq<TAO_PEGTL_STRING("#"), pg::xdigit, pg::xdigit, pg::xdigit, pg::xdigit>,
	pg::seq<TAO_PEGTL_STRING("#"), pg::xdigit, pg::xdigit, pg::xdigit>> {};

// medium <- IDENT S*
// medium-list <- medium ("," S* medium)*
// media <- "@media" S* medium "{" S* stylesheet "}"

} // namespace parser

export class StylePropertyMap {
public:
	void parse(const std::string_view content, const std::string &source)
	{
		pg::memory_input in(content.data(), content.size(), source);
		pg::parse<parser::decls, parser::action>(in, *this);
	}
	fifo_map<base::string_atom, Value> values;
};

} // namespace style
