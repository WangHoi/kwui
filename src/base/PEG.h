#pragma once
#include "absl/status/statusor.h"
#include "absl/strings/str_split.h"
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

inline IResult<std::string_view> spaces(absl::string_view input)
{
	int i;
	for (i = 0; i < input.length(); ++i) {
		if (!std::isspace(input[i]))
			break;
	}
	if (i == 0) {
		return absl::InvalidArgumentError(input);
	} else {
		return std::make_tuple(input.substr(i), input.substr(0, i));
	}
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

} // namespace peg
} // namespace base
