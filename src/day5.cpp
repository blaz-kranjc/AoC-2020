#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <string>
#include <vector>
#include <bitset>

constexpr auto n_rows = 128;
constexpr auto n_columns = 8;

constexpr auto bisector(char upper_code, char lower_code, int length)
{
  return [=, limits = std::pair{ 0, length-1 }] (std::string_view code) mutable {
    while (!code.empty()) {
      if (int sector_length = limits.second - limits.first; code[0] == lower_code) {
        code = code.substr(1);
        limits = { limits.first, limits.second - (sector_length + 1) / 2 };
      } else if (code[0] == upper_code) {
        code = code.substr(1);
        limits = { limits.first + (sector_length + 1) / 2, limits.second };
      } else {
        return -1;
      }
    }
    return limits.first;
  };
}

int seat_id(std::string_view pass)
{
  auto row_of = bisector('B', 'F', n_rows);
  auto column_of = bisector('R', 'L', n_columns);
  return row_of(pass.substr(0, 7)) * n_columns + column_of(pass.substr(7));
}

std::vector<std::string> parse(std::istream &&is)
{
  std::vector<std::string> result;
  std::string data;
  while (!is.eof()) {
    is >> data;
    if (!data.empty()) {
      result.push_back(std::move(data));
    }
  }
  return result;
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));

  const auto seat_ids = data | ranges::views::transform([](const auto &s) { return seat_id(s); });
  fmt::print("Part1: {}\n", *ranges::max_element(seat_ids));

  const auto part2 = [&] {
    std::bitset<n_rows*n_columns> occupied;
    ranges::for_each(seat_ids, [&](auto id) { occupied.set(id); });
    auto begin = 0;
    for (; !occupied.test(begin); ++begin) {}
    for (; occupied.test(begin); ++begin) {}
    return begin;
  }();
  fmt::print("Part2: {}\n", part2);
}
