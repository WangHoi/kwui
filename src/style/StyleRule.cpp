#include "StyleRule.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_split.h"
#include <cctype>

namespace style {

namespace {

template <typename T>
using IResult = std::tuple<std::string_view, T>;

template <typename T>
struct StatusOrType {};

template <typename T>
struct StatusOrType<absl::StatusOr<std::tuple<std::string_view, T>>> {
    using value_type = typename T;
};

template <typename F1, typename F2>
auto seq(F1&& f1, F2&& f2) {
    using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
    using T2 = StatusOrType<typename std::result_of<F2(std::string_view)>::type>::value_type;
    return [f1, f2](absl::string_view input) -> absl::StatusOr<IResult<std::tuple<T1, T2>>> {
        auto res1 = f1(input);
        if (!res1.ok())
            return res1.status();
        auto [output1, val1] = std::move(*res1);

        auto res2 = f2(output1);
        if (!res2.ok())
            return res2.status();
        auto [output2, val2] = std::move(*res2);
        return IResult<std::tuple<T1, T2>>(output2, std::make_tuple<T1, T2>(std::move(val1), std::move(val2)));
        };
}

template <typename F1, typename F2>
auto alt(F1&& f1, F2&& f2) {
    using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
    using T2 = StatusOrType<typename std::result_of<F2(std::string_view)>::type>::value_type;
    static_assert(std::is_same<T1, T2>::value, "alt type mismatch error.");
    return [f1, f2](absl::string_view input) -> absl::StatusOr<IResult<T1>> {
        auto res1 = f1(input);
        if (res1.ok())
            return res1;

        return f2(input);
        };
}

template <typename F1>
auto opt(F1&& f1) {
    using T1 = StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
    return [f1](absl::string_view input) -> absl::StatusOr<IResult<absl::optional<T1>>> {
        auto res1 = f1(input);
        if (!res1.ok())
            return IResult<std::optional<T1>>(input, std::nullopt);
        auto [output1, val1] = std::move(*res1);
        return IResult<std::optional<T1>>(output1, absl::make_optional<T1>(std::move(val1)));
        };
}

template <typename F1, typename TF>
auto map(F1&& f1, TF&& tf) {
    using T1 = typename StatusOrType<typename std::result_of<F1(std::string_view)>::type>::value_type;
    using T2 = typename std::result_of<TF(T1)>::type;
    return [f1, tf](absl::string_view input) -> absl::StatusOr<IResult<T2>> {
        auto res1 = f1(input);
        if (!res1.ok())
            return absl::StatusOr<IResult<T2>>(res1.status());
        auto&& [output1, t1] = *res1;
        auto t2 = tf(t1);
        return IResult<T2>(output1, t2);
        };
}

absl::StatusOr<IResult<std::string_view>> spaces(absl::string_view input)
{
    int i;
    for (i = 0; i < input.length(); ++i) {
        if (!std::isspace(input[i]))
            break;
    }
    if (i == 0) {
        return absl::InvalidArgumentError(input);
    } else {
        return IResult<std::string_view>(input.substr(i), input.substr(0, i));
    }
}

auto tag(absl::string_view pattern)
{
    return [pattern](absl::string_view input) -> absl::StatusOr<IResult<std::string_view>> {
        if (absl::StartsWith(input, pattern)) {
            return IResult<std::string_view>(input.substr(pattern.length()), input.substr(0, pattern.length()));
        } else {
            return absl::InvalidArgumentError("invalid tag");
        }
        };
}

absl::StatusOr<IResult<std::string_view>> token(absl::string_view input)
{
    int i;
    for (i = 0; i < input.length(); ++i) {
        if (i == 0 && !std::isalpha(input[i]))
            break;
        else if (!std::isalnum(input[i]) && input[i] != '_' && input[i] != '-')
            break;
    }
    if (i == 0) {
        return absl::InvalidArgumentError(input);
    } else {
        return IResult<std::string_view>(input.substr(i), input.substr(0, i));
    }
}

absl::StatusOr<IResult<std::unique_ptr<Selector>>> selector_item(absl::string_view input)
{
    auto [output1, _] = *opt(spaces)(input);

    Selector sel;
    auto id = seq(tag("#"), token);
    auto klass = seq(tag("#"), token);

    int count = 0;
    int n = 0;
    do {
        n = 0;

        auto res1 = map(id, [&sel, &n](auto tup) {
            sel.id = base::string_intern(std::string(std::get<1>(tup))); 
            return ++n;
            })(input);
        std::tie(input, std::ignore) = res1.value_or(std::make_tuple(input, n));

        auto res2 = map(klass, [&sel, &n](auto tup) {
            sel.klass = Classes::parse(std::get<1>(tup)); 
            return ++n;
            })(input);
        std::tie(input, std::ignore) = res2.value_or(std::make_tuple(input, n));

        auto res3 = map(token, [&sel, &n](auto s) {
            sel.tag = base::string_intern(std::string(s)); 
            return ++n;
            })(input);
        std::tie(input, std::ignore) = res3.value_or(std::make_tuple(input, n));

        auto res4 = map(tag("*"), [&sel, &n](auto s) {
            sel.tag = base::string_intern("*"); 
            return ++n;
            })(input);
        std::tie(input, std::ignore) = res4.value_or(std::make_tuple(input, n));

        count += n;
    } while (n > 0);

    if (count > 0)
        return std::make_tuple(input, std::make_unique<Selector>(std::move(sel)));
    else
        return absl::InvalidArgumentError("parse Selector error");
}

absl::StatusOr<IResult<SelectorDependency>> combinator(absl::string_view input)
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

    return IResult<SelectorDependency>(output1, dep);
}

absl::StatusOr<IResult<std::unique_ptr<Selector>>> selector(absl::string_view input)
{
    auto sel1_res = selector_item(input);
    if (!sel1_res.ok())
        return sel1_res.status();
    auto&& [output1, sel1] = *sel1_res;
    std::unique_ptr<Selector> sel =std::move(sel1);

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

absl::StatusOr<IResult<std::vector<std::unique_ptr<Selector>>>> selector_group(absl::string_view input)
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

}

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
    if (tag != base::string_atom())
        s += 1;
    if (dep_selector)
        s += dep_selector->specificity();
    return s;
}

}