#ifndef _AST_HPP_
#define _AST_HPP_

#include "../util/string.hpp"

#include <array>
#include <memory>
#include <string_view>
#include <variant>

namespace cxlisp::ast {

// Arbitrary, can be increased
constexpr size_t kMaxDepth = 1024;
constexpr size_t kMaxListSize = 1024;
constexpr size_t kMaxDottedListSize = 1024;

template<size_t Depth=kMaxDepth>
struct Value {

    union Data {
        std::string_view atom;
        std::array<Value<Depth - 1>, kMaxListSize> list;
        std::array<Value<Depth - 1>, kMaxDottedListSize> dotted_list;
        bool boolean;
        int integer;
        util::String string;
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
    Data value;

    constexpr static auto makeAtom(std::string_view atom) -> Value<Depth> {
        Value<Depth> value;
        value.type = Type::kAtom;
        value.value.atom = atom;
        return value;
    }

    constexpr static auto makeList(std::array<Value<Depth - 1>, kMaxListSize> list) -> Value<Depth> {
        Value<Depth> value;
        value.type = Type::kList;
        value.value.list = list;
        return value;
    }

    constexpr static auto makeDottedList(std::array<Value<Depth - 1>, kMaxDottedListSize> dotted_list) -> Value<Depth> {
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
template<> struct Value<0> {};

}  // namespace cxlisp::ast

#endif  // _AST_HPP_