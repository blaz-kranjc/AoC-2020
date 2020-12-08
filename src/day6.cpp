#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <vector>
#include <array>

using group_t = std::pair<int, std::array<int, 26>>;

std::vector<group_t> parse(std::istream &&is)
{
  std::vector<group_t> results;

  int count{ 0 };
  std::array<int, 26> group{ 0 };
  char current;
  char previous{ 0 };
  while (!is.eof()) {
    current = is.get();
    if (!std::islower(current) && !std::islower(previous)) {
      results.emplace_back(count, std::move(group));
      count = 0;
      group.fill(0);
    } else if (std::islower(current)) {
      ++group[current - 'a'];
    } else {
      ++count;
    }
    std::swap(current, previous);
  }
  return results;
}

std::size_t count_distinct_yes(const group_t &g)
{
  return ranges::count_if(g.second, [](auto e) { return e > 0; });
}

std::size_t count_group_yes(const group_t &g)
{
  return ranges::count_if(g.second, [n = g.first](auto e) { return e == n; });
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));

  fmt::print("Part 1: {}\n", ranges::accumulate(data | ranges::views::transform(count_distinct_yes), 0ll));
  fmt::print("Part 2: {}\n", ranges::accumulate(data | ranges::views::transform(count_group_yes), 0ll));
}
