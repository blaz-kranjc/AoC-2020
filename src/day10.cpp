#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

std::unordered_set<int> parse(std::istream &&is)
{
  std::unordered_set<int> result;
  int el;
  while (is >> el) {
    result.insert(el);
  }
  return result;
}

// Max is at most 3*N, so this is O(N)
std::array<int, 3> count_gaps(const std::unordered_set<int>& data, int max)
{
  std::array<int, 3> gaps{ 0 };
  int current = 0;
  for (int i = 1; i <= max; ++i) {
    if (data.contains(i)) {
      ++gaps[i - current - 1];
      current = i;
    }
  }
  // Add the last gap of size 3
  ++gaps[2];
  return gaps;
}

long long n_arrangements(const std::unordered_set<int> &data, int max)
{
  std::unordered_map<int, long long> cache;
  const auto impl = [&](auto &&f, int index) {
    if (const auto it = cache.find(index); it != cache.cend()) {
      return it->second;
    }
    if (index == max) {
      cache.emplace(index, 1);
      return 1ll;
    }
    long long value = 0;
    for (int i = index + 1; i <= std::min(max, index + 3); ++i) {
      if (data.contains(i)) {
        value += f(f, i);
      }
    }
    cache.emplace(index, value);
    return value;
  };
  return impl(impl, 0);
}

int main(int argc, char **argv)
{
  auto data = parse(load_input(argc, argv));

  auto max = *ranges::max_element(data);
  const auto gaps = count_gaps(data, max);
  fmt::print("Part 1: {}\n", gaps[0] * gaps[2]);
  fmt::print("Part 2: {}\n", n_arrangements(data, max));
}