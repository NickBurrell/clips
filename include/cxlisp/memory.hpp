#ifndef CXLISP_MEMORY_HPP
#define CXLISP_MEMORY_HPP

#include "cxlisp.hpp"

namespace cxlisp::memory {

template <typename> class AllocatorCommon;
template <typename> struct AllocTraits;

template <typename T, typename TAllocator,
          typename = typename util::has_type<
              T, typename AllocTraits<TAllocator>::SupportedAllocTypes>>
class UniquePointer {
public:
  constexpr UniquePointer() = default;

  constexpr UniquePointer(UniquePointer &&lhs) {
    m_parent_ = lhs.m_parent_;
    m_alloc_idx_ = lhs.m_alloc_idx_;
    m_is_populated_ = lhs.m_is_populated_;
  }

  constexpr UniquePointer(TAllocator &parent, std::size_t idx)
      : m_is_populated_(true), m_alloc_idx_(idx), m_parent_(parent) {}

  constexpr void populate(T t) {
    auto new_idx = m_parent_.reserve();
    m_parent_.insert_at(t, new_idx);
    m_is_populated_ = true;
  }

  constexpr const T *borrow() {
    return m_parent_.template read<T>(m_alloc_idx_);
  }
  constexpr T *borrow_mut() {
    return m_parent_.template read_mut<T>(m_alloc_idx_);
  };

  constexpr T operator*() {
    return m_parent_.template release<T>(m_alloc_idx_);
  }

private:
  bool m_is_populated_{};
  std::size_t m_alloc_idx_{};
  std::optional<TAllocator *> m_parent_{};
};

template <typename Child> class AllocatorCommon {
public:
  template <typename T,
            typename = util::has_type<
                T, typename AllocTraits<Child>::SupportedAllocTypes>,
            typename... Args>
  [[nodiscard]] constexpr auto allocate(Args... args) {
    return static_cast<Child *>(this)->template allocate_impl<T>(args...);
  }
  template <typename T,
            typename = util::has_type<
                T, typename AllocTraits<Child>::SupportedAllocTypes>,
            typename... Args>
  [[nodiscard]] constexpr auto reserve() {
    return static_cast<Child *>(this)->template reserve_impl<T>();
  }
  template <typename T,
            typename = util::has_type<
                T, typename AllocTraits<Child>::SupportedAllocTypes>>
  [[nodiscard]] constexpr auto release(std::size_t idx) {
    return static_cast<Child *>(this)->template release_impl<T>(idx);
  }
  template <typename T,
            typename = util::has_type<
                T, typename AllocTraits<Child>::SupportedAllocTypes>>
  constexpr void remove(std::size_t idx) {
    return static_cast<Child *>(this)->template remove_impl<T>(idx);
  }
};

template <typename T, std::size_t kMaxElements>
class Allocator : public AllocatorCommon<Allocator<T, kMaxElements>> {
public:
  typedef std::tuple<T> SupportedAllocTypes;

  template <typename... Args>
  constexpr UniquePointer<T, Allocator<T, kMaxElements>>
  allocate_impl(Args... args) {
    auto entry_idx = find_next_free();
    if (!entry_idx)
      throw std::runtime_error("no free entries");
    m_data_[*entry_idx] = T{args...};
    m_free_list_[*entry_idx] = false;
    return UniquePointer{*this, *entry_idx};
  }

  constexpr UniquePointer<T, Allocator<T, kMaxElements>> reserve_impl() {
    auto entry_idx = find_next_free();
    if (!entry_idx)
      throw std::runtime_error("no free entries");
    m_free_list_[*entry_idx] = false;
    return UniquePointer<T, Allocator<T, kMaxElements>>::new_uninhabited(
        *this, entry_idx);
  }

  constexpr T release_impl(std::size_t idx) {
    if (idx > kMaxElements)
      throw std::runtime_error("index out of range");
    if (m_free_list_[idx])
      throw std::runtime_error("attempt to release free entry");
    T val = std::move(m_data_[idx]);
    m_free_list_[idx] = true;

    return val;
  }

  constexpr void remove_impl(std::size_t idx) {

    if (idx > kMaxElements)
      throw std::runtime_error("index out of range");
    if (m_free_list_[idx])
      throw std::runtime_error("attempt to release free entry");
    m_free_list_[idx] = true;
  }

private:
  constexpr std::optional<std::size_t> inline find_next_free() {
    for (auto *e = m_free_list_.cbegin(); e != m_free_list_.cend(); ++e) {
      if (*e)
        return static_cast<std::size_t>(e - m_free_list_.cbegin());
    }
    return std::nullopt;
  }

  constexpr UniquePointer<T, Allocator<T, kMaxElements>>
  insert_at(T val, std::size_t idx) {
    if (idx > kMaxElements)
      throw std::runtime_error("index out of range");
    if (!m_free_list_[idx])
      throw std::runtime_error("attempt to release free entry");
    m_data_[idx] = val;
    m_free_list_[idx] = false;
    return UniquePointer{idx, &this};
  }

  constexpr T *read_mut(std::size_t idx) {
    if (idx > kMaxElements)
      throw std::runtime_error("index out of range");
    if (m_free_list_[idx])
      throw std::runtime_error("attempt to release free entry");
    return (m_data_.begin() + idx);
  }

  std::array<T, kMaxElements> m_data_{};
  std::array<bool, kMaxElements> m_free_list_{true};
  friend class UniquePointer<T, Allocator<T, kMaxElements>>;
};

template <typename T, std::size_t kMaxSize>
struct AllocTraits<Allocator<T, kMaxSize>> {
  typedef std::tuple<T> SupportedAllocTypes;
};

} // namespace cxlisp::memory
#endif // CXLISP_MEMORY_HPP