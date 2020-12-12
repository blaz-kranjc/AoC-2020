#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <unordered_set>
#include <iostream>

std::unordered_set<int> parse(std::istream&& is)
{
  std::unordered_set<int> result;
  int el;
  while (is >> el) {
    result.insert(el);
  }
  return result;
}

int main(int argc, char** argv)
{
  const auto data = parse(load_input(argc, argv));

  const auto part1 = [&] {
    for (auto e : data) {
      if (data.contains(2020 - e)) {
        return e * (2020 - e);
      }
    }
    return -1;
  }();
  fmt::print("Part 1: {}\n", part1);

  const auto part2 = [&] {
    for (auto e : data) {
      for (auto f : data | ranges::views::drop(1)) {
        const auto residual = 2020 - e - f;
        if (data.find(residual) != data.end()) {
          return residual * e * f;
        }
      }
    }
    return -1;
  }();
  fmt::print("Part 2: {}\n", part2);
}