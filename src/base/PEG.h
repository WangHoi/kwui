#pragma once
#include "absl/status/statusor.h"
#include "absl/strings/str_split.h"
#include "absl/strings/numbers.h"
#include <cctype>

namespace base {
namespace peg {

template <typename T>
using IResult = absl::StatusOr<std::tuple<std::string_view, T>>;

namespace {
template <typename T>
struct StatusOrType {};

template <typename T>
struct StatusOrType<IResult<T>> {
	using value_type = typename T;
};
}

template <typename F1, typename F2>
inline auto seq(F1&& f1, F2&& f2) {
	using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
	using T2 = StatusOrType<typename std::result_of<F2(std::string_view)>::type>::value_type;
	return [f1, f2](absl::string_view input) -> IResult<std::tuple<T1, T2>> {
		auto res1 = f1(input);
		if (!res1.ok())
			return res1.status();
		auto [output1, val1] = std::move(*res1);

		auto res2 = f2(output1);
		if (!res2.ok())
			return res2.status();
		auto [output2, val2] = std::move(*res2);
		return std::make_tuple(output2, std::make_tuple<T1, T2>(std::move(val1), std::move(val2)));
		};
}

template <typename F1, typename F2, typename F3>
inline auto seq(F1&& f1, F2&& f2, F3&& f3) {
	using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
	using T2 = StatusOrType<typename std::result_of<F2(std::string_view)>::type>::value_type;
	using T3 = StatusOrType<typename std::result_of<F3(std::string_view)>::type>::value_type;
	return [f1, f2, f3](absl::string_view input) -> IResult<std::tuple<T1, T2, T3>> {
		auto res1 = f1(input);
		if (!res1.ok())
			return res1.status();
		auto [output1, val1] = std::move(*res1);

		auto res2 = f2(output1);
		if (!res2.ok())
			return res2.status();
		auto [output2, val2] = std::move(*res2);
		
		auto res3 = f3(output2);
		if (!res3.ok())
			return res3.status();
		auto [output3, val3] = std::move(*res3);

		return std::make_tuple(output3, std::make_tuple<T1, T2, T3>(std::move(val1), std::move(val2), std::move(val3)));
		};
}

template <typename F1, typename F2, typename F3, typename F4>
inline auto seq(F1&& f1, F2&& f2, F3&& f3, F4&& f4) {
	using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
	using T2 = StatusOrType<typename std::result_of<F2(std::string_view)>::type>::value_type;
	using T3 = StatusOrType<typename std::result_of<F3(std::string_view)>::type>::value_type;
	using T4 = StatusOrType<typename std::result_of<F4(std::string_view)>::type>::value_type;
	return [f1, f2, f3, f4](absl::string_view input) -> IResult<std::tuple<T1, T2, T3, T4>> {
		auto res1 = f1(input);
		if (!res1.ok())
			return res1.status();
		auto [output1, val1] = std::move(*res1);

		auto res2 = f2(output1);
		if (!res2.ok())
			return res2.status();
		auto [output2, val2] = std::move(*res2);

		auto res3 = f3(output2);
		if (!res3.ok())
			return res3.status();
		auto [output3, val3] = std::move(*res3);

		auto res4 = f4(output3);
		if (!res4.ok())
			return res4.status();
		auto [output4, val4] = std::move(*res4);

		return std::make_tuple(output4, std::make_tuple<T1, T2, T3, T4>(std::move(val1), std::move(val2), std::move(val3), std::move(val4)));
		};
}

template <typename F1, typename F2>
inline auto alt(F1&& f1, F2&& f2) {
	using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
	using T2 = StatusOrType<typename std::result_of<F2(std::string_view)>::type>::value_type;
	static_assert(std::is_same<T1, T2>::value, "alt type mismatch error.");
	return [f1, f2](absl::string_view input) -> IResult<T1> {
		auto res1 = f1(input);
		if (res1.ok())
			return res1;

		return f2(input);
		};
}
template <typename F1, typename F2, typename F3>
inline auto alt(F1&& f1, F2&& f2, F3&& f3) {
	using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
	using T2 = StatusOrType<typename std::result_of<F2(std::string_view)>::type>::value_type;
	using T3 = StatusOrType<typename std::result_of<F3(std::string_view)>::type>::value_type;
	static_assert(std::is_same<T1, T2>::value, "alt type mismatch error.");
	static_assert(std::is_same<T1, T3>::value, "alt type mismatch error.");
	return [f1, f2, f3](absl::string_view input) -> IResult<T1> {
		auto res1 = f1(input);
		if (res1.ok())
			return res1;

		auto res2 = f2(input);
		if (res2.ok())
			return res2;

		return f3(input);
		};
}
template <typename F1, typename F2, typename F3, typename F4>
inline auto alt(F1&& f1, F2&& f2, F3&& f3, F4&& f4) {
	using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
	using T2 = StatusOrType<typename std::result_of<F2(std::string_view)>::type>::value_type;
	using T3 = StatusOrType<typename std::result_of<F3(std::string_view)>::type>::value_type;
	using T4 = StatusOrType<typename std::result_of<F4(std::string_view)>::type>::value_type;
	static_assert(std::is_same<T1, T2>::value, "alt type mismatch error.");
	static_assert(std::is_same<T1, T3>::value, "alt type mismatch error.");
	static_assert(std::is_same<T1, T4>::value, "alt type mismatch error.");
	return [f1, f2, f3, f4](absl::string_view input) -> IResult<T1> {
		auto res1 = f1(input);
		if (res1.ok())
			return res1;

		auto res2 = f2(input);
		if (res2.ok())
			return res2;

		auto res3 = f3(input);
		if (res3.ok())
			return res3;
		
		return f4(input);
		};
}

template <typename F1>
inline auto opt(F1&& f1) {
	using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
	return [f1](absl::string_view input) -> IResult<absl::optional<T1>> {
		auto res1 = f1(input);
		if (!res1.ok())
			return std::make_tuple(input, std::nullopt);
		auto [output1, val1] = std::move(*res1);
		return std::make_tuple(output1, absl::make_optional<T1>(std::move(val1)));
		};
}

template <typename F1, typename TF>
inline auto map(F1&& f1, TF&& tf) {
	using T1 = typename StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
	using T2 = typename std::result_of<TF(T1)>::type;
	return [f1, tf](absl::string_view input) -> IResult<T2> {
		auto res1 = f1(input);
		if (!res1.ok())
			return IResult<T2>(res1.status());
		auto&& [output1, t1] = *res1;
		auto t2 = tf(t1);
		return std::make_tuple(output1, t2);
		};
}

template <typename F>
inline auto take_while_m_n(F&& f, int m, int n)
{
	return [f, m, n](absl::string_view input) -> IResult<std::string_view> {
		int i;
		for (i = 0; i < input.length() && i < n; ++i) {
			if (!f(input[i]))
				break;
		}
		if (i < m) {
			return absl::InvalidArgumentError(input);
		} else {
			return std::make_tuple(input.substr(i), input.substr(0, i));
		}
		};
}

template <typename F>
inline auto take_while(F&& f)
{
	return take_while_m_n(f, 0, INT_MAX);
}

template <typename F>
inline auto take_while1(F&& f)
{
	return take_while_m_n(f, 1, INT_MAX);
}

template <typename SEP, typename F>
inline auto separated_list0(SEP&& sep, F&& f)
{
	using T = StatusOrType<typename std::result_of<F(std::string_view)>::type>::value_type;
	return [sep, f](absl::string_view input) -> IResult<std::vector<T>> {
		std::vector<T> list;
		absl::string_view output = input;
		auto res = f(input);
		while (res.ok()) {
			auto&& [output1, t] = res.value();
			list.push_back(t);
			output = output1;
			
			auto res2 = sep(output1);
			if (!res2.ok()) {
				break;
			}
			std::tie(output1, std::ignore) = res2.value();
			res = f(input);
		}
		return std::make_tuple(output, list);
		};
}

template <typename SEP, typename F>
inline auto separated_list1(SEP&& sep, F&& f)
{
	using T = StatusOrType<typename std::result_of<F(std::string_view)>::type>::value_type;
	return [sep, f](absl::string_view input) -> IResult<std::vector<T>> {
		std::vector<T> list;
		absl::string_view output = input;
		auto res = f(input);

		if (!res.ok())
			return res.status();

		while (res.ok()) {
			auto&& [output1, t] = res.value();
			list.push_back(t);
			output = output1;

			auto res2 = sep(output1);
			if (!res2.ok()) {
				break;
			}
			std::tie(output1, std::ignore) = res2.value();
			res = f(input);
		}
		return std::make_tuple(output, list);
		};
}

// Syntax: SPACE_CHAR+
inline IResult<std::string_view> spaces(absl::string_view input)
{
	return take_while1(std::isspace)(input);
}

inline auto tag(absl::string_view pattern)
{
	return [pattern](absl::string_view input) -> IResult<std::string_view> {
		if (absl::StartsWith(input, pattern)) {
			return std::make_tuple(input.substr(pattern.length()), input.substr(0, pattern.length()));
		} else {
			return absl::InvalidArgumentError("invalid tag");
		}
		};
}

/* Syntax:
 *		nmstart	<- [ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_]
 *		nmchar	<- nmstart / [0123456789-]
 *		ident	<- nmstart nmchar*
 */
inline IResult<std::string_view> ident(absl::string_view input)
{
	int i;
	for (i = 0; i < input.length(); ++i) {
		if (i == 0 && !std::isalpha(input[i]) && input[i] != '_')
			break;
		else if (!std::isalnum(input[i]) && input[i] != '_' && input[i] != '-')
			break;
	}
	if (i == 0) {
		return absl::InvalidArgumentError(input);
	} else {
		return std::make_tuple(input.substr(i), input.substr(0, i));
	}
}

// Syntax: [0123456789]+
inline IResult<absl::string_view> digits(absl::string_view input)
{
	int i;
	for (i = 0; i < input.length(); ++i) {
		if (!std::isdigit(input[i]))
			break;
	}
	if (i == 0) {
		return absl::InvalidArgumentError(input);
	} else {
		return std::make_tuple(input.substr(i), input.substr(0, i));
	}
}

// Syntax: ("-" | "+")? [0123456789]* "."? [0123456789]*
inline IResult<float> number(absl::string_view input)
{
	auto res = seq(
		alt(tag("-"), tag("+")),
		opt(digits),
		opt(tag(".")),
		opt(digits)
	)(input);
	if (!res.ok())
		return res.status();
	
	auto&& [output, val] = res.value();

	float f32;
	bool ok = absl::SimpleAtof(input.substr(0, input.length() - output.length()), &f32);
	if (ok)
		return std::make_tuple(output, f32);
	else
		return absl::InvalidArgumentError(input);
}

} // namespace peg
} // namespace base
