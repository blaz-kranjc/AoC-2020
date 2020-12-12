#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
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
  cache.emplace(max, 1ll);
  const auto impl = [&](auto &&f, int index) {
    if (const auto it = cache.find(index); it != cache.cend()) {
      return it->second;
    } else {
      const auto value = ranges::accumulate(
        ranges::views::iota(index + 1, std::min(max, index + 3) + 1)
          | ranges::views::filter([&](auto i) { return data.contains(i); })
          | ranges::views::transform([&](auto i) { return f(f, i); }),
        0ll
      );
      cache.emplace(index, value);
      return value;
    }
  };
  return impl(impl, 0);
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));

  const auto max = *ranges::max_element(data);
  const auto gaps = count_gaps(data, max);
  fmt::print("Part 1: {}\n", gaps[0] * gaps[2]);
  fmt::print("Part 2: {}\n", n_arrangements(data, max));
}