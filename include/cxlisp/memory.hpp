#ifndef CXLISP_MEMORY_HPP
#define CXLISP_MEMORY_HPP

namespace cxlisp::memory {

template <typename, std::size_t> class UniquePointer;

template <typename T, std::size_t kMaxElements> class IFixedAllocator {
  constexpr virtual UniquePointer<T, kMaxElements> reserve() = 0;
  constexpr virtual T release(std::size_t) = 0;
  constexpr virtual void remove(std::size_t) = 0;
};

template <typename T, std::size_t kMaxElements>
class Allocator : IFixedAllocator<T, kMaxElements> {
public:
  template <typename... Args>
  constexpr UniquePointer<T, kMaxElements> allocate(Args... args) {
    auto entry_idx = find_next_free();
    if (!entry_idx)
      throw std::runtime_error("no free entries");
    m_data_[*entry_idx] = T{args...};
    m_free_list_[*entry_idx] = false;
    return UniquePointer{*this, *entry_idx};
  }

  constexpr UniquePointer<T, kMaxElements> reserve() {
    auto entry_idx = find_next_free();
    if (!entry_idx)
      throw std::runtime_error("no free entries");
    m_free_list_[*entry_idx] = false;
    return UniquePointer<T, kMaxElements>::new_uninhabited(*this, entry_idx);
  }

  constexpr T release(std::size_t idx) {
    if (idx > kMaxElements)
      throw std::runtime_error("index out of range");
    if (m_free_list_[idx])
      throw std::runtime_error("attempt to release free entry");
    T val = std::move(m_data_[idx]);
    m_free_list_[idx] = true;

    return val;
  }

  constexpr void remove(std::size_t idx) {

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

  constexpr UniquePointer<T, kMaxElements> insert_at(T val, std::size_t idx) {
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
  friend class UniquePointer<T, kMaxElements>;
};

template <typename T, std::size_t kMaxElements> class UniquePointer {
public:
  constexpr UniquePointer(IFixedAllocator<T, kMaxElements> &parent,
                          std::size_t idx)
      : m_is_populated_(true), m_alloc_idx_(idx), m_parent_(parent) {}

  constexpr static UniquePointer<T, kMaxElements>
  new_uninhabited(IFixedAllocator<T, kMaxElements> &parent, std::size_t idx) {
    auto val = UniquePointer(parent, idx);
    val.m_is_populated_ = false;
  }

  constexpr void populate(T t) {
    auto new_idx = m_parent_.reserve();
    m_parent_.insert_at(t, new_idx);
    m_is_populated_ = true;
  }

  constexpr const T *borrow() { return m_parent_.read(m_alloc_idx_); }
  constexpr T *borrow_mut() { return m_parent_.read_mut(m_alloc_idx_); };

  constexpr explicit operator T() { return m_parent_.release(m_alloc_idx_); }

private:
  bool m_is_populated_;
  std::size_t m_alloc_idx_;
  IFixedAllocator<T, kMaxElements> &m_parent_;
};

} // namespace cxlisp::memory
#endif // CXLISP_MEMORY_HPP