#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <string>
#include <cinttypes>
#include <vector>
#include <bitset>

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
  }
  return result;
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", *ranges::max_element(data));

  const auto empty_seat = [&] {
    std::bitset<128 * 8> taken_seats;
    ranges::for_each(data, [&](auto el) { taken_seats.set(el); });
    std::uint64_t empty{ 0 };
    for (; !taken_seats.test(empty); ++empty) {}
    for (; taken_seats.test(empty); ++empty) {}
    return empty;
  }();
  fmt::print("Part 2: {}\n", empty_seat);
}
