#ifndef CXLISP_UTIL_TYPE_TRAITS_HPP
#define CXLISP_UTIL_TYPE_TRAITS_HPP

#include <tuple>
#include <type_traits>

namespace cxlisp::util {
template <typename T, typename Tuple> struct has_type;

template <typename T> struct has_type<T, std::tuple<>> : std::false_type {};

template <typename T, typename U, typename... Ts>
struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

template <typename T, typename... Ts>
struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

template <typename T, typename Tuple>
using has_type_t = typename has_type<T, Tuple>::type;

};     // namespace cxlisp::util
#endif // CXLISP_UTIL_TYPE_TRAITS_HPP
