#ifndef _VECTOR_HPP_
#define _VECTOR_HPP_

#include <array>
namespace cxlisp::util {
template <typename TBase, size_t kMaxSize,
          typename T = std::remove_cv_t<std::remove_pointer_t<TBase>>>
class Vector {
public:
  using iterator = typename std::array<T, kMaxSize>::iterator;
  using const_iterator = typename std::array<T, kMaxSize>::const_iterator;
  using value_type = T;
  using reference = typename std::array<T, kMaxSize>::reference;
  using const_reference = typename std::array<T, kMaxSize>::const_reference;

  constexpr Vector() = default;

  template <typename TIter>
  constexpr Vector(TIter begin, const TIter &end) : m_data_({}), m_size_(0) {
    for (auto d_begin = m_data_.begin(); begin != end; ++begin, ++d_begin) {
      *d_begin = *begin;
    }
  }

  constexpr Vector(std::initializer_list<T> list) : m_size_(0) {
    for (auto &value : list) {
      push_back(std::move(value));
    }
  }

  constexpr auto push_back(T &&value) -> T & {
    if (m_size_ == kMaxSize)
      throw std::runtime_error(
          "Vector is full"); // This is ok, since throwing an exception errors
                             // out in a constexpr context
    T &t = m_data_[m_size_++];
    t = std::move(value);
    return t;
  }

  constexpr auto pop_back() -> T {
    if (m_size_ == 0)
      throw std::runtime_error("Vector is empty");
    return std::move(m_data_[--m_size_]);
  }

  constexpr auto insert(iterator s_begin, iterator s_end) {
    /*if (m_size_ + static_cast<std::size_t>(s_end - s_begin) > kMaxSize)
      throw std::runtime_error("Vector is full");*/
    while (s_begin != s_end) {
      push_back(std::move(*s_begin));
      ++s_begin;
    }
  }

  constexpr auto begin() { return m_data_.begin(); }
  constexpr auto end() { return m_data_.end(); }

  constexpr auto cbegin() const { return m_data_.begin(); }
  constexpr auto cend() const { return m_data_.end(); }

  constexpr const auto &operator[](std::size_t index) const {
    return m_data_[index];
  }
  constexpr auto &operator[](std::size_t index) { return m_data_[index]; }

  constexpr auto size() const { return m_size_; }

  constexpr auto &back() { return m_data_[m_size_ - 1]; }
  constexpr const auto &back() const { return m_data_[m_size_ - 1]; }

  constexpr auto &front() { return m_data_[0]; }
  constexpr const auto &front() const { return m_data_[0]; }

  constexpr auto empty() const { return m_size_ == 0; }
  constexpr auto full() const { return m_size_ == kMaxSize; }

  constexpr void clear() { m_size_ = 0; }

  constexpr auto data() { return m_data_.data(); }

  constexpr auto back_insert_iter() { return m_data_.begin() + m_size_ - 1; }

private:
  std::array<T, kMaxSize> m_data_;
  std::size_t m_size_;
};
} // namespace cxlisp::util

#endif /* _VECTOR_HPP_ */