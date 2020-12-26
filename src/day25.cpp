#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>

std::pair<long long, long long> parse(std::istream&& is) {
  long long k1;
  is >> k1;
  long long k2;
  is >> k2;
  return std::pair{ k1, k2 };
}

long long round(long long begin, int base, int times = 1) {
  for (int i = 0; i < times; ++i) {
    begin = (begin * base) % 20201227;
  }
  return begin;
}

int find_rounds(long long n) {
  int i = 0;
  for (long long c = 1; c != n; c = round(c, 7)) {
    ++i;
  }
  return i;
}

int main(int argc, char **argv)
{
  const auto [k1, k2] = parse(load_input(argc, argv));
  const long long k1_rounds = find_rounds(k1);
  fmt::print("Part 1: {}\n", round(1, k2, k1_rounds));
}
