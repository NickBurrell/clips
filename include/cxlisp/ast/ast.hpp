
/*******************************************************************************
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 ******************************************************************************/

#ifndef CXLISP_AST_HPP
#define CXLISP_AST_HPP

#include "cxlisp/memory.hpp"
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
template <std::size_t kMaxNodes = kMaxValueNodes> class ValueAllocator {

  using Value =
      std::variant<memory::UniquePointer<struct Atom, kMaxNodes>,
                   memory::UniquePointer<struct List, kMaxNodes>,
                   memory::UniquePointer<struct DottedList, kMaxNodes>,
                   memory::UniquePointer<struct Number, kMaxNodes>,
                   memory::UniquePointer<struct String, kMaxNodes>,
                   memory::UniquePointer<struct Bool, kMaxNodes>>;

  template <typename TBase> struct ValueBase {
    constexpr auto get() { return static_cast<TBase *>(this)->value; }
  };

  struct Atom : ValueBase<Atom> {
    util::String value;
  };

  struct List : ValueBase<List> {
    util::Vector<memory::UniquePointer<Value, kMaxNodes>, kMaxListLength> value;
  };

  struct DottedList : ValueBase<DottedList> {
    util::Vector<memory::UniquePointer<Value, kMaxNodes>, kMaxDottedListLength>
        value{};
  };

  struct Number : ValueBase<Number> {
    constexpr explicit Number(uint64_t v) : value(v) {}
    constexpr Number() = default;
    uint64_t value{};
  };

  struct Bool : ValueBase<Bool> {
    bool value{};
  };

private:
  class ValueAllocatorInner : public memory::IFixedAllocator<Value, kMaxNodes> {
  public:
    template <typename T, typename... Args>
    constexpr Value allocate(Args... args) {
      if constexpr (std::is_same_v<T, Atom>) {
        return m_atom_allocator_.allocate(args...);
      } else if constexpr (std::is_same_v<T, List>) {
        return m_list_allocator_.allocate(args...);
      } else if constexpr (std::is_same_v<T, DottedList>) {
        return m_dotted_list_allocator_.allocate(args...);
      } else if constexpr (std::is_same_v<T, Number>) {
        return m_number_allocator_.allocate(args...);
      } else if constexpr (std::is_same_v<T, Bool>) {
        return m_bool_allocator_.allocate(args...);
      } else {
        throw std::runtime_error("invalid type allocation");
      }
    }

    template <typename T> memory::UniquePointer<Value, kMaxNodes> reserve() {
      return memory::UniquePointer<Value, kMaxNodes>::new_uninhabited(this);
    }

    template <typename T> Value release() {
      if constexpr (std::is_same_v<T, Atom>) {
        return m_atom_allocator_.release();
      } else if constexpr (std::is_same_v<T, List>) {
        return m_list_allocator_.release();
      } else if constexpr (std::is_same_v<T, DottedList>) {
        return m_dotted_list_allocator_.release();
      } else if constexpr (std::is_same_v<T, Number>) {
        return m_number_allocator_.release();
      } else if constexpr (std::is_same_v<T, Bool>) {
        return m_bool_allocator_.release();
      } else {
        throw std::runtime_error("invalid type allocation");
      }
    }

    template <typename T> void remove() {
      if constexpr (std::is_same_v<T, Atom>) {
        return m_atom_allocator_.remove();
      } else if constexpr (std::is_same_v<T, List>) {
        return m_list_allocator_.remove();
      } else if constexpr (std::is_same_v<T, DottedList>) {
        return m_dotted_list_allocator_.remove();
      } else if constexpr (std::is_same_v<T, Number>) {
        return m_number_allocator_.remove();
      } else if constexpr (std::is_same_v<T, Bool>) {
        return m_bool_allocator_.remove();
      } else {
        throw std::runtime_error("invalid type allocation");
      }
    }
    constexpr virtual Value release(std::size_t size) override {
      return nullptr;
    }
    constexpr virtual void remove(std::size_t size) override {}

  private:
    memory::Allocator<Atom, kMaxNodes> m_atom_allocator_{};
    memory::Allocator<List, kMaxNodes> m_list_allocator_{};
    memory::Allocator<DottedList, kMaxNodes> m_dotted_list_allocator_{};
    memory::Allocator<Number, kMaxNodes> m_number_allocator_{};
    memory::Allocator<Bool, kMaxNodes> m_bool_allocator_{};
  };

private:
  ValueAllocatorInner m_inner_alloc_{};
  friend struct Atom;
  friend struct List;
  friend struct DottedList;
  friend struct Number;
  friend struct Bool;
};

} // namespace cxlisp::ast

#endif // CXLISP_AST_HPP