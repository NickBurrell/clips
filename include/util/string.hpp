#ifndef _STRING_HPP_
#define _STRING_HPP_

#include <string_view>

#include "vector.hpp"
namespace cxlisp::util {
template <typename Char, std::size_t kMaxSize>
class BasicString : public Vector<Char, kMaxSize> {
 public:
  constexpr BasicString() = default;

  constexpr BasicString(std::string_view sv)
      : Vector<Char, kMaxSize>(sv.cbegin(), sv.cend()) {}
  constexpr BasicString& operator=(std::string_view sv) {
    Vector<Char, kMaxSize>::clear();
    for (auto& c : sv) {
      this->push_back(c);
    }
    return *this;
  }

  constexpr BasicString(const char* str, std::size_t size)
      : Vector<Char, kMaxSize>(str, str + size) {
    for (auto i = 0u; i < size; ++i) {
      this->push_back(std::forward<char>(const_cast<char*>(str)[i]));
    }
  }

  constexpr auto operator==(const BasicString& rhs) const {
    const char* lhs_iter = this->cbegin();
    const char* rhs_iter = rhs.cbegin();
    while (lhs_iter != this->cend() && rhs_iter != rhs.cend() &&
           (*lhs_iter == *rhs_iter)) {
      ++lhs_iter, ++rhs_iter;
    }
    return lhs_iter == this->cend() && rhs_iter == rhs.cend();
  }

  constexpr const char* c_str() { return this->data(); }
};

using String = BasicString<char, 1024>;

constexpr auto operator""_cxs(const char* str, std::size_t size) {
  return String(str, size);
}
}  // namespace cxlisp::util
#endif /* _STRING_HPP_ */