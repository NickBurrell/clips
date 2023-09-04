
/*******************************************************************************
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 ******************************************************************************/

#ifndef CXLISP_AST_HPP
#define CXLISP_AST_HPP

#include "cxlisp/util/util.hpp"

#include <array>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <variant>
#include <vector>

namespace cxlisp::ast {

// Arbitrary, can be increased
constexpr size_t kMaxValueNodes = 0x10000;
constexpr size_t kMaxListLength = 0x10000;
constexpr size_t kMaxDottedListLength = 0x10000;

using Value = std::variant<struct Atom *, struct List *, struct DottedList *,
                           struct Number *, struct String *, struct Bool *>;

template <typename TBase> struct ValueBase {
  constexpr auto get() { return static_cast<TBase *>(this)->value; }
};

struct Atom : ValueBase<Atom> {
  util::String value{};
};

struct List : ValueBase<List> {
  util::Vector<Value *, kMaxListLength> value{};
};

struct DottedList : ValueBase<DottedList> {
  util::Vector<Value *, kMaxDottedListLength> value{};
};

struct Number : ValueBase<Number> {
  constexpr explicit Number(uint64_t v) : value(v) {}
  constexpr Number() = default;
  uint64_t value{0};
};

struct Bool : ValueBase<Bool> {
  bool value{false};
};

template <std::size_t kMaxAllocSize = kMaxValueNodes> class ValueAllocator {

  class AllocEntry;

public:
  template <typename T, typename... Args>
  constexpr auto allocate(Args... args) {
    if constexpr (std::is_same_v<T, Atom>) {
      // return m_atom_allocator_.allocate(args...);
    } else if constexpr (std::is_same_v<T, List>) {
      // return m_list_allocator_.allocate(args...);
    } else if constexpr (std::is_same_v<T, DottedList>) {
      // return m_dotted_list_allocator_.allocate(args...);
    } else if constexpr (std::is_same_v<T, Number>) {
      return m_number_allocator_.allocate(args...);
    } else if constexpr (std::is_same_v<T, Bool>) {
      return m_bool_allocator_.allocate(args...);
    } else {
      throw std::runtime_error("invalid type allocation");
    }
  }

  template <typename T> AllocEntry release() {
    if constexpr (std::is_same_v<T, Atom>) {
      // return m_atom_allocator_.release();
    } else if constexpr (std::is_same_v<T, List>) {
      // return m_list_allocator_.release();
    } else if constexpr (std::is_same_v<T, DottedList>) {
      // return m_dotted_list_allocator_.release();
    } else if constexpr (std::is_same_v<T, Number>) {
      return m_number_allocator_.release();
    } else if constexpr (std::is_same_v<T, Bool>) {
      return m_bool_allocator_.release();
    } else {
      throw std::runtime_error("invalid type allocation");
    }
  }

private:
  template <typename T, std::size_t kMaxElements> class Allocator {
  public:
    class AllocEntry {
    public:
      constexpr AllocEntry(Allocator &parent, std::size_t idx)
          : m_alloc_idx_(idx), m_parent_(parent) {}

      constexpr const T *borrow() { return m_parent_.read(m_alloc_idx_); }
      constexpr T *borrow_mut() { return m_parent_.read_mut(m_alloc_idx_); };

      constexpr explicit operator T() {
        return m_parent_.release(m_alloc_idx_);
      }

    private:
      std::size_t m_alloc_idx_;
      Allocator &m_parent_;
    };

    template <typename... Args> constexpr AllocEntry allocate(Args... args) {
      auto entry_idx = find_next_free();
      if (!entry_idx)
        throw std::runtime_error("no free entries");
      m_data_[*entry_idx] = T(args...);
      m_free_list_[*entry_idx] = false;
      return AllocEntry{*this, *entry_idx};
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

    template <typename... Args>
    constexpr AllocEntry allocate_at(std::size_t idx, Args... args) {
      auto val = T(args...);
      insert_at(idx, args...);
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

    constexpr AllocEntry insert_at(T val, std::size_t idx) {
      if (idx > kMaxElements)
        throw std::runtime_error("index out of range");
      if (!m_free_list_[idx])
        throw std::runtime_error("attempt to release free entry");
      m_data_[idx] = val;
      m_free_list_[idx] = false;
      return AllocEntry{idx, &this};
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
    friend class AllocEntry;
  };

  // Allocator<Atom, kMaxAllocSize> m_atom_allocator_{};
  // Allocator<List, kMaxAllocSize> m_list_allocator_{};
  Allocator<DottedList, kMaxAllocSize> m_dotted_list_allocator_{};
  Allocator<Number, kMaxAllocSize> m_number_allocator_{};
  Allocator<Bool, kMaxAllocSize> m_bool_allocator_{};
};

} // namespace cxlisp::ast

#endif // CXLISP_AST_HPP