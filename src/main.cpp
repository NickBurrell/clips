#include <iostream>

#include "colisp.hpp"

using namespace cxlisp::util;
using namespace cxlisp::parser;

using namespace std::literals;
static_assert(parseString()("\"test\""sv)->first == "test"_cxs);
static_assert(parseAtom()("test"sv)->first == "test"_cxs);
int main() {
  std::cout << "Hello, from CoLisp!\n";
  std::cout << "test"_cxs.size() << std::endl;
  return 0;
}
