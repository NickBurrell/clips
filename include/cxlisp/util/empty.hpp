#ifndef _EMPTY_HPP_
#define _EMPTY_HPP_
namespace cxlisp::util {
struct EmptyType {
  constexpr EmptyType() = default;
  constexpr EmptyType(const EmptyType &) = default;
};
} // namespace cxlisp::util
#endif /* _EMPTY_HPP_ */