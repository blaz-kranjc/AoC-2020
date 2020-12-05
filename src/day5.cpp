#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <string>
#include <cinttypes>
#include <vector>

constexpr std::uint64_t seat_id(std::string_view pass) {
  std::uint64_t id{ 0 };
  for (; !pass.empty(); pass = pass.substr(1)) {
    id <<= 1;
    id |= static_cast<uint64_t>(pass[0] == 'B' || pass[0] == 'R');
  }
  return id;
}

std::vector<std::uint64_t> parse(std::istream &&is)
{
  std::vector<std::uint64_t> result;
  std::string data;
  while (!is.eof()) {
    is >> data;
    if (!data.empty()) {
      result.push_back(seat_id(data));
    }
    data.clear();
  }
  return result;
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));
  const auto [min_it, max_it] = ranges::minmax_element(data);
  fmt::print("Part 1: {}\n", *max_it);

  const auto expected_full_sum = ((*min_it + *max_it) * (*max_it - *min_it + 1)) / 2;
  const auto sum = ranges::accumulate(data, 0ull);
  fmt::print("Part 2: {}\n", expected_full_sum - sum);
}
