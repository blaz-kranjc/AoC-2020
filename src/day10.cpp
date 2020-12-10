#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <vector>
#include <iostream>

std::vector<int> parse(std::istream &&is)
{
  std::vector<int> result;
  long long el;
  while (is >> el) {
    result.push_back(el);
  }
  return result;
}

long long n_arrangements(const std::vector<int> &data)
{
  std::vector<long long> cache(data.size(), -1);
  const auto impl = [&](auto &&f, std::size_t index) {
    if (cache[index] >= 0) {
      return cache[index];
    }
    if (index == data.size() - 1) {
      cache[index] = 1;
      return 1ll;
    }
    long long value = 0;
    for (int i = index + 1; i < data.size() && data[i] - data[index] <= 3; ++i) {
        value += f(f, i);
    }
    cache[index] = value;
    return value;
  };
  long long value = 0;
  for (int i = 0; data[i] <= 3; ++i) {
    value += impl(impl, i);
  }
  return value;
}

int main(int argc, char **argv)
{
  auto data = parse(load_input(argc, argv));

  ranges::sort(data);
  auto diffs = ranges::accumulate(
    ranges::views::zip(data, data | ranges::views::drop(1)),
    std::array<int, 3>{},
    [](auto acc, auto el) {
      ++acc[el.second - el.first - 1];
      return acc;
    });
  ++diffs[data[0] - 1];
  ++diffs[2];
  fmt::print("Part 1: {}\n", diffs[0] * diffs[2]);
  fmt::print("Part 2: {}\n", n_arrangements(data));
}