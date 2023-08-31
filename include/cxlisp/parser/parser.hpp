
/*******************************************************************************
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 ******************************************************************************/

#ifndef CXLISP_PARSER_HPP_
#define CXLISP_PARSER_HPP_
#include <optional>
#include <string_view>

#include "cxlisp/util/string.hpp"

namespace cxlisp::parser {

// Inspired by constexpr_all_the_things.
// Source:
// https://github.com/lefticus/constexpr_all_the_things/blob/master/src/include/cx_parser.h

template <typename TParser>
using Result = std::optional<std::pair<TParser, std::string_view>>;

template <typename TParser>
using ParseResultPair =
    typename std::invoke_result_t<TParser, std::string_view>::value_type;

template <typename TParser>
using Parser = typename ParseResultPair<TParser>::first_type;

namespace ops {
template <typename TFunc, typename TParser>
constexpr auto fmap(TFunc &&f, TParser &&p) {
  using TResult = Result<std::invoke_result_t<TFunc, Parser<TParser>>>;
  return [f = std::forward<TFunc>(f),
          p = std::forward<TParser>(p)](std::string_view str) -> TResult {
    const auto result = p(str);
    if (!result)
      return std::nullopt;
    return TParser(std::make_pair(f(result->first), result->second));
  };
}

template <typename TParser, typename TFunc>
constexpr auto bind(TParser &&p, TFunc &&f) {
  using TResult =
      std::invoke_result_t<TFunc, Parser<TParser>, std::string_view>;
  return [=](std::string_view str) -> TResult {
    const auto result = p(str);
    if (!result)
      return std::nullopt;
    return f(result->first, result->second);
  };
}

template <typename TFunc, typename TParser>
constexpr auto operator>>=(TParser &&p, TFunc &&f) {
  return bind(std::forward<TParser>(p), std::forward<TFunc>(f));
}

template <typename TVal> constexpr auto pure(TVal v) {
  return [=](std::string_view str) -> Result<TVal> {
    return std::make_pair(std::move(v), str);
  };
}

template <typename TFunc> constexpr auto lift(TFunc &&f) {
  return [=](std::string_view str) -> Result<std::invoke_result_t<TFunc>> {
    return std::make_pair(f(str), str);
  };
}

template <typename TVal> constexpr auto fail(TVal) {
  return [=](auto) -> Result<TVal> { return std::nullopt; };
}

template <typename TVal, typename TErr> constexpr auto fail(TVal, TErr e) {
  return [=](auto) -> Result<TVal> {
    e();
    return std::nullopt;
  };
}

template <typename TParser1, typename TParser2>
constexpr auto operator|(TParser1 &&p1, TParser2 &&p2) {
  return [=](Parser<TParser1> str) -> Result<Parser<TParser1>> {
    const auto result = p1(str);
    if (result)
      return result;
    return p2(str);
  };
}
} // namespace ops
namespace combinators {
template <typename TParser1, typename TParser2, typename TFunc,
          typename TParser =
              std::invoke_result_t<TFunc, Parser<TParser1>, Parser<TParser2>>>
constexpr auto accumulate(TParser1 &&p1, TParser2 &&p2, TFunc &&f) {
  return [=](Parser<TParser1> str) -> Result<TParser> {
    const auto result1 = p1(str);
    if (!result1)
      return std::nullopt;
    const auto result2 = p2(result1->second);
    if (!result2)
      return std::nullopt;
    return std::make_pair(f(result1->first, result2->first), result2->second);
  };
}

template <typename TParser1, typename TParser2, typename = Parser<TParser1>,
          typename = Parser<TParser2>>
constexpr auto operator<(TParser1 &&p1, TParser2 &&p2) {
  return accumulate(std::forward<TParser1>(p1), std::forward<TParser2>(p2),
                    [](const auto &a, auto) { return a; });
}

template <typename TParser1, typename TParser2, typename = Parser<TParser1>,
          typename = Parser<TParser2>>
constexpr auto operator>(TParser1 &&p1, TParser2 &&p2) {
  return accumulate(std::forward<TParser1>(p1), std::forward<TParser2>(p2),
                    [](auto, const auto &b) { return b; });
}

template <typename TParser> constexpr auto zeroOrOne(TParser &&p) {
  using TVal = Parser<TParser>;
  return [p = std::forward<TParser>(p)](std::string_view str) -> Result<TVal> {
    const auto result = p(str);
    if (result)
      return result;
    return std::make_pair(TVal(), str);
  };
}

namespace detail {

template <typename TParser, typename TAcc, typename TFunc>
constexpr std::pair<TAcc, std::string_view>
foldl(std::string_view str, TParser p, TAcc acc, TFunc &&f) {
  for (auto c = str.begin(); c != str.end();) {
    const auto result = p(c);
    if (!result)
      return std::make_pair(acc, str);
    acc = f(acc, result->first);
    str = result->second;
    c = str.begin();
  }
  return std::make_pair(acc, str);
}

template <typename TParser, typename TAcc, typename TFunc>
constexpr std::pair<TAcc, std::string_view>
foldlN(std::string_view str, TParser p, std::size_t n, TAcc acc, TFunc &&f) {
  while (n != 0) {
    const auto result = p(str);
    if (!result)
      return std::make_pair(acc, str);
    acc = f(acc, result->first);
    str = result->second();
    --n;
  }
}
} // namespace detail

template <typename TParser, typename TAcc, typename TFunc>
constexpr auto many(TParser &&p, TAcc &&acc, TFunc &&f) {
  return [p = std::forward<TParser>(p), acc = std::forward<TAcc>(acc),
          f = std::forward<TFunc>(f)](std::string_view str) {
    return Result<TAcc>(detail::foldl(str, p, acc, f));
  };
}

template <typename TParser, typename TAcc, typename TFunc>
constexpr auto many1(TParser &&p, TAcc &&acc, TFunc &&f) {
  return [p = std::forward<TParser>(p), acc = std::forward<TAcc>(acc),
          f = std::forward<TFunc>(f)](std::string_view str) -> Result<TAcc> {
    const auto result = p(str);
    if (!result)
      return std::nullopt;
    return Result<TAcc>(
        detail::foldl(result->second, p, f(acc, result->first), f));
  };
}

template <typename TParser, typename TAcc, typename TFunc>
constexpr auto exactlyN(TParser &&p, std::size_t n, TAcc &&acc, TFunc &&f) {
  return [p = std::forward<TParser>(p), acc = std::forward<TAcc>(acc),
          f = std::forward<TFunc>(f), n](std::string_view str) {
    return Result<TAcc>(detail::foldlN(str, p, n, acc, f));
  };
}

template <typename TParser, typename T = Parser<TParser>>
constexpr auto option(TParser &&p, T &&def) {
  return [def = std::forward<T>(def),
          p = std::forward<TParser>(p)](std::string_view str) {
    const auto result = p(str);
    if (!result)
      return std::make_pair(def, str);
    return result;
  };
}

template <typename TParser1, typename TParser2, typename TFunc>
constexpr auto separatedBy(TParser1 &&p1, TParser2 &&p2, TFunc &&f) {
  using TVal = Parser<TParser1>;
  return [p1 = std::forward<TParser1>(p1), p2 = std::forward<TParser2>(p2),
          f = std::forward<TFunc>(f)](std::string_view str) -> Result<TVal> {
    const auto result = p1(str);
    if (!result)
      return std::nullopt;
    const auto p = p2 < p1;
    return Result<TVal>(detail::foldl(result->second, p, result->first, f));
  };
}

template <typename TParser1, typename TParser2, typename TFunc, typename TInit>
constexpr auto separatedBy(TParser1 &&p1, TParser2 &&p2, TInit &&acc,
                           TFunc &&f) {
  using TVal = Result<std::invoke_result_t<TInit>>;
  return [p1 = std::forward<TParser1>(p1), p2 = std::forward<TParser2>(p2),
          init = std::forward<TInit>(acc),
          f = std::forward<TFunc>(f)](std::string_view str) -> Result<TVal> {
    const auto result = p1(str);
    if (!result)
      return std::make_pair(init, str);
    const auto p = p2 < p1;
    return Result<TVal>(
        detail::foldl(result->second, p, f(init(), result->first), f));
  };
}
template <typename TParser1, typename TParser2, typename TFunc, typename TVal>
constexpr auto separatedByValue(TParser1 &&p1, TParser2 &&p2, TVal &&v,
                                TFunc &&f) {
  using TResult = Result<std::remove_reference_t<TVal>>;
  return [p1 = std::forward<TParser1>(p1), p2 = std::forward<TParser2>(p2),
          v = std::forward<TVal>(v),
          f = std::forward<TFunc>(f)](std::string_view str) -> TResult {
    const auto result = p1(str);
    if (!result)
      return std::make_pair(v, str);
    const auto p = p2 < p1;
    return Result<TVal>(
        detail::foldl(result->second, p, f(v, result->first), f));
  };
}

} // namespace combinators

constexpr auto makeCharParser(char c) {
  return [=](std::string_view sv) -> Result<char> {
    if (sv.empty())
      return std::nullopt;
    if (sv[0] == c)
      return std::make_pair(c, sv.substr(1));
    return std::nullopt;
  };
}

constexpr auto oneOf(std::string_view chars) {
  return [=](std::string_view sv) -> Result<char> {
    if (sv.empty())
      return std::nullopt;
    for (auto c : chars) {
      if (sv[0] == c)
        return std::make_pair(c, sv.substr(1));
    }
    return std::nullopt;
  };
}

constexpr auto noneOf(std::string_view chars) {
  return [=](std::string_view sv) -> Result<char> {
    if (sv.empty())
      return std::nullopt;
    for (auto c : chars) {
      if (sv[0] == c)
        return std::nullopt;
    }
    return std::make_pair(sv[0], sv.substr(1));
  };
}

constexpr auto makeStringParser(std::string_view str) {
  return [=](auto sv) -> Result<std::string_view> {
    if (str.empty())
      return std::nullopt;
    if (str.substr(0, str.size()) == sv)
      return std::make_pair(str, str.substr(sv.size()));
    return std::nullopt;
  };
}
/**
 * Value Parser
 */

using namespace std::literals;
constexpr auto skipWhitespace = combinators::many(
    oneOf(" \t\n\r"sv), std::monostate{}, [](auto l, auto) { return l; });

constexpr auto parseString() {
  using namespace std::literals;
  using namespace cxlisp::util;

  return parser::ops::bind(makeCharParser('"'), [](auto, auto str1) {
    return parser::ops::bind(combinators::many(noneOf("\""sv), String{},
                                               [](auto acc, auto c) {
                                                 acc.push_back(std::move(c));
                                                 return acc;
                                               }),
                             [](auto str, auto rest) {
                               return parser::ops::bind(
                                   makeCharParser('"'), [=](auto, auto rest) {
                                     return ops::pure(str)(rest);
                                   })(rest);
                             })(str1);
  });
}

constexpr auto parseAtom() {
  using namespace std::literals;
  using namespace cxlisp::util;

  return parser::ops::bind(
      combinators::many1(noneOf(" \t\n\r()\";"sv), String{},
                         [](auto acc, auto c) {
                           acc.push_back(std::move(c));
                           return acc;
                         }),
      [](auto str, auto rest) {
        return parser::ops::bind(skipWhitespace, [=](auto, auto rest) {
          return ops::pure(str)(rest);
        })(rest);
      });
}

constexpr auto numberParser() {
  using namespace std::literals;
  return combinators::many1(oneOf("0123456789"sv), 0, [](auto acc, char c) {
    return acc * 10 + (c - '0');
  });
}

}; // namespace cxlisp::parser

#endif /* CXLISP_PARSER_HPP_ */