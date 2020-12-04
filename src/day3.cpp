#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <functional>
#include <string>

std::vector<std::string> parse(std::istream &&is)
{
  std::vector<std::string> result;
  for (std::string line; std::getline(is, line);) {
    result.push_back(line);
  }
  return result;
}

int count_hits(const std::vector<std::string> data, std::pair<int, int> slope)
{
  int count = 0;
  const auto n_columns = data[0].size();
  for (int row = 0; row < data.size(); row += slope.first) {
    if (data[row][((row / slope.first) * slope.second) % n_columns] == '#') ++count;
  }
  return count;
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));

  fmt::print("Part 1: {}\n", count_hits(data, std::pair{ 1, 3 }));

  const auto part2 = [&] {
    constexpr std::array slopes{
      std::pair{ 1, 1 },
      std::pair{ 1, 3 },
      std::pair{ 1, 5 },
      std::pair{ 1, 7 },
      std::pair{ 2, 1 },
    };
    return ranges::accumulate(
      slopes | ranges::views::transform([&](auto slope) { return count_hits(data, slope); }),
      1,
      std::multiplies{});
  }();
  fmt::print("Part 2: {}\n", part2);
}
