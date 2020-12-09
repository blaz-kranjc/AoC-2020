#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <vector>
#include <iostream>

std::vector<long long> parse(std::istream&& is)
{
  std::vector<long long> result;
  long long el;
  while (is >> el) {
    result.push_back(el);
  }
  return result;
}

template <typename Iter, typename T>
bool contains_sum(Iter begin, Iter end, const T& value) {
  for (; begin + 1 != end; ++begin) {
    for (auto it = begin + 1; it != end; ++it) {
      if (*begin + *it == value) return true;
    }
  }
  return false;
}

int main(int argc, char** argv)
{
  auto data = parse(load_input(argc, argv));

  const auto part1 = [&] {
    auto begin = data.cbegin();
    auto end = data.cbegin() + 25;
    while (end != data.cend()) {
      if (!contains_sum(begin, end, *end)) {
        return *end;
      }
      ++begin;
      ++end;
    }
    return -1ll;
  }();
  fmt::print("Part 1: {}\n", part1);

  const auto part2 = [&, part1] {
    auto begin = data.cbegin();
    auto end = data.cbegin() + 1;
    int sum = *begin;
    while (end != data.end()) {
      if (sum == part1) {
        auto mm = ranges::minmax_element(begin, end);
        return *mm.min + *mm.max;
      } else if (sum < part1) {
        sum += *end;
        ++end;
      } else {
        sum -= *begin;
        ++begin;
      }
    }

    return -1ll;
  }();
  fmt::print("Part 2: {}\n", part2);
}