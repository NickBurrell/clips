
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
#include <string_view>
#include <variant>

namespace cxlisp::ast {

// Arbitrary, can be increased
constexpr size_t kMaxDepth = 64;
constexpr size_t kMaxListSize = 1024;
constexpr size_t kMaxDottedListSize = 1024;

template <size_t Depth = kMaxDepth> struct Value {

  constexpr Value() : type(Type::kUnassigned), empty(util::EmptyType{}) {
    static_assert(Depth > 0, "Depth must be greater than 0");
  }
  union {
    util::String atom;
    std::array<Value<Depth - 1>, kMaxListSize> list;
    std::array<Value<Depth - 1>, kMaxDottedListSize> dotted_list;
    bool boolean;
    int integer;
    util::String string;
    util::EmptyType empty;
  };

  enum struct Type {
    kUnassigned,
    kAtom,
    kList,
    kDottedList,
    kBoolean,
    kInteger,
    kString
  };

  Type type = Type::kUnassigned;

  constexpr static auto makeAtom(util::String atom) -> Value<Depth> {
    Value<Depth> value;
    value.type = Type::kAtom;
    value.value.atom = atom;
    return value;
  }

  constexpr static auto
  makeList(std::array<Value<Depth - 1>, kMaxListSize> list) -> Value<Depth> {
    Value<Depth> value;
    value.type = Type::kList;
    value.value.list = list;
    return value;
  }

  constexpr static auto
  makeDottedList(std::array<Value<Depth - 1>, kMaxDottedListSize> dotted_list)
      -> Value<Depth> {
    Value<Depth> value;
    value.type = Type::kDottedList;
    value.value.dotted_list = dotted_list;
    return value;
  }

  constexpr static auto makeBoolean(bool boolean) -> Value<Depth> {
    Value<Depth> value;
    value.type = Type::kBoolean;
    value.value.boolean = boolean;
    return value;
  }

  constexpr static auto makeInteger(int integer) -> Value<Depth> {
    Value<Depth> value;
    value.type = Type::kInteger;
    value.value.integer = integer;
    return value;
  }

  constexpr static auto makeString(util::String string) -> Value<Depth> {
    Value<Depth> value;
    value.type = Type::kString;
    value.value.string = string;
    return value;
  }
};

// Base case
template <> struct Value<0> {};

} // namespace cxlisp::ast

#endif // _AST_HPP_