#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <regex>
#include <bit>

static const std::regex mem_line{ R"(mem\[(\d+)\] = (\d+))"};

struct Mask
{
  std::bitset<36> ones;
  std::bitset<36> floats;
  std::bitset<36> zeroes() const {
    std::bitset<36> result;
    result.set();
    result &= ~ones;
    result &= ~floats;
    return result;
  }
};

struct Memory
{
  std::size_t mask_index;
  std::uint64_t addr;
  std::uint64_t value;
};

Mask parse_mask(std::string_view str) {
  Mask result;
  for (auto [i, c] : str | ranges::views::reverse | ranges::views::enumerate) {
    if (c == '1') {
      result.ones.set(i);
    } else if (c == 'X') {
      result.floats.set(i);
    }
  }
  return result;
}

struct Program
{
  std::vector<Mask> masks;
  std::vector<Memory> memory;
};

Program parse(std::istream&& is)
{
  std::vector<Mask> masks;
  std::vector<Memory> memory;

  for (std::string d; std::getline(is, d);) {
    if (d.starts_with("mask = ")) {
      masks.push_back(parse_mask(std::string_view{ d.begin() + 7, d.begin() + 7 + 36 }));
    } else {
      std::smatch match;
      if (!std::regex_match(d, match, mem_line)) {
        throw std::runtime_error{ "Mismatch in memory input" };
      }
      memory.emplace_back(masks.size() - 1, std::stoull(match[1]), std::stoull(match[2]));
    }
  }
  return Program{ .masks = std::move(masks), .memory = std::move(memory) };
}

std::uint64_t part_1(const Program& program) {
  std::unordered_map<std::uint64_t, std::uint64_t> values;
  for (const auto m : program.memory) {
    const auto &mask = program.masks[m.mask_index];
    values[m.addr] = m.value & (~mask.zeroes().to_ullong()) | mask.ones.to_ullong();
  }
  std::uint64_t sum = 0;
  for (const auto [k, v] : values) {
    sum += v;
  }
  return sum;
}

std::uint64_t part_2(const Program& program) {
  std::unordered_map<std::uint64_t, std::uint64_t> values;
  for (const auto m : program.memory) {
    const auto &mask = program.masks[m.mask_index];
    std::vector<std::uint64_t> keys{ m.addr | mask.ones.to_ullong() };
    keys.reserve(1ULL << mask.floats.count());
    for (int i = 0; i < 36; ++i) {
      if (mask.floats.test(i)) {
        std::transform(keys.cbegin(), keys.cend(), std::back_inserter(keys), [i](auto k) { return k ^ (1ULL << i); });
      }
    }
    for (const auto k : keys) {
      values[k] = m.value;
    }
  }

  std::uint64_t sum = 0;
  for (const auto [k, v] : values) {
    sum += v;
  }
  return sum;
}

int main(int argc, char **argv)
{
  const auto program = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", part_1(program));
  fmt::print("Part 2: {}\n", part_2(program));
}
