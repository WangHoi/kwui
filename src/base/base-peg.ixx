module;
#include <optional>
#include <expected>
#include <tuple>
#include <string_view>
#include <iostream>
export module base:peg;

namespace base {
namespace peg {

export enum class ErrorKind {
	Tag,
};

export class ParseError {
public:
	static ParseError fromErrorKind(std::string_view input, ErrorKind kind)
	{
		return ParseError();
	}
	static ParseError append(std::string_view input, ErrorKind kind, ParseError other)
	{
		return ParseError();
	}
	static ParseError or_(ParseError other)
	{
		return ParseError();
	}
};

export template <typename Self>
concept parse_errorify = requires (Self other, std::string_view input, ErrorKind kind) {
	{ Self::fromErrorKind(input, kind) } -> std::same_as<Self>;
	{ Self::append(input, kind, other) } -> std::same_as<Self>;
	{ Self::or_(other) } -> std::same_as<Self>;
};

template <typename I, typename E, typename O, typename F>
concept applicable = std::regular_invocable<F, I>&& std::same_as<F(I), std::expected<std::tuple<O, I>, E>>;

export template <typename E = ParseError>
	requires parse_errorify<E>
auto tag(std::string_view s)
{
	return [s](std::string_view input) -> std::expected<std::tuple<std::string_view, std::string_view>, E> {
		if (input.starts_with(s)) {
			std::cout << input << std::endl << s << std::endl;
			return std::make_tuple(input.substr(0, s.length()), input.substr(s.length()));
		} else {
			return std::unexpected(E::fromErrorKind(input, ErrorKind::Tag));
		}
		};
}

template <typename F>
struct apply_fn {};

template <typename I, typename O, typename E>
struct apply_fn<std::expected<std::tuple<I, O>, E>> {
	using input_type = I;
	using value_type = O;
	using error_type = E;
	using result_type = std::expected<std::tuple<I, O>, E>;
};

export template <typename F1, typename F2, typename E = ParseError>
struct seq {
	F1 f1;
	F2 f2;
	seq(F1 f1_, F2 f2_): f1(f1_), f2(f2_) {
	}

	auto operator()(std::string_view input) {
		using I = std::string_view;
		using R1 = apply_fn<std::invoke_result_t<F1, I>>::result_type;
		using O1 = apply_fn<std::invoke_result_t<F1, I>>::value_type;
		using R2 = apply_fn<std::invoke_result_t<F2, I>>::result_type;
		using O2 = apply_fn<std::invoke_result_t<F2, I>>::value_type;
		using Ret = std::expected<std::tuple<I, std::tuple<O1, O2> >, E>;
		
		Ret ret;

		R1 res1 = f1(input);
		if (!res1) {
			return Ret(std::unexpected(res1.error()));
		}
		auto&& [i1, o1] = std::move(*res1);

		R2 res2 = f2(i1);
		if (!res2) {
			//return res2.transform(Ret);
		}

		auto&& [i2, o2] = std::move(*res2);
		return Ret(std::make_tuple(i2, std::make_tuple(o1, o2)));
	}
};

/*
template <typename I, typename O1, typename O2, typename E, typename F1, typename F2>
	requires applicable<I, E, O1, F1>&& applicable<I, E, O2, F2>
std::expected<std::tuple<std::tuple<O1, O2>, I>, E> tuple(F1 f1, F2 ff)
{
	return [f1, ff](I input) -> std::expected<std::tuple<std::tuple<O1, O2>, I>, E> {
		std::expected<std::tuple<O1, I>, E> res1 = f1(input);
		if (!res1) {
			return res1;
		}
		auto&& [o1, ri1] = std::move(*res1);

		std::expected<std::tuple<O2, I>, E> res2 = ff(ri1);
		if (!res2) {
			return res2;
		}
		auto&& [o2, ri2] = std::move(*res2);
		return std::make_tuple<std::tuple<O1, O2>, I>(
			std::make_tuple<O1, O2>(std::move(o1), std::move(o2)),
			ri2);
		};
}
*/
template <typename I, typename E, typename F1, typename O, typename F2>
	requires applicable<I, E, O, F1>&& applicable<I, E, O, F2>
std::expected<std::tuple<O, I>, E> alt(F1 f1, F2 ff)
{
	return [&f1, &ff](I input) -> std::expected<std::tuple<O, I>, E> {
		std::expected<std::tuple<O, I>, E> res1 = f1(input);
		if (res1) {
			return res1;
		}

		std::expected<std::tuple<O, I>, E> res2 = ff(input);
		if (res2) {
			return res2;
		}
		auto&& [t2, o2] = std::move(*res2);
		return res2;
		};
}


}
}