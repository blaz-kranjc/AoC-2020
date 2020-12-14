#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <unordered_map>
#include <bitset>
#include <regex>

static const std::regex mem_line{ R"(mem\[(\d+)\] = (\d+))"};

struct Instruction
{
  std::uint64_t addr;
  std::uint64_t value;
  std::bitset<36> mask;
  std::bitset<36> float_mask;
};

std::pair<std::bitset<36>, std::bitset<36>> parse_mask(std::string_view str)
{
  std::bitset<36> mask;
  std::bitset<36> float_mask;
  for (auto [i, c] : str | ranges::views::reverse | ranges::views::enumerate) {
    if (c == '1') {
      mask.set(i);
    } else if (c == 'X') {
      float_mask.set(i);
    }
  }
  return std::pair{ mask, float_mask };
}

std::vector<Instruction> parse(std::istream&& is)
{
  std::vector<Instruction> instructions;
  std::bitset<36> mask;
  std::bitset<36> float_mask;

  for (std::string d; std::getline(is, d);) {
    if (d.starts_with("mask = ")) {
      std::tie(mask, float_mask) = parse_mask(std::string_view{ d.begin() + 7, d.begin() + 7 + 36 });
    } else {
      std::smatch match;
      if (!std::regex_match(d, match, mem_line)) {
        throw std::runtime_error{ "Mismatch in memory input" };
      }
      instructions.emplace_back(std::stoull(match[1]), std::stoull(match[2]), mask, float_mask);
    }
  }
  return instructions;
}

std::uint64_t part_1(const std::vector<Instruction>& program) {
  std::unordered_map<std::uint64_t, std::uint64_t> values;
  ranges::for_each(
    program,
    [&](const auto &m) {
      values[m.addr] = (m.value & m.float_mask.to_ullong()) | m.mask.to_ullong();
    });
  return ranges::accumulate(
    values | ranges::views::transform([](const auto &v) { return v.second; }),
    0ULL
  );
}

std::vector<std::uint64_t> generate_values(std::uint64_t value, std::bitset<36> float_mask) {
  std::vector<std::uint64_t> keys{ value };
  keys.reserve(1ULL << float_mask.count());
  for (int i = 0; i < float_mask.size(); ++i) {
    if (float_mask.test(i)) {
      std::transform(keys.cbegin(), keys.cend(), std::back_inserter(keys), [i](auto k) { return k ^ (1ULL << i); });
    }
  }
  return keys;
}

std::uint64_t part_2(const std::vector<Instruction>& program) {
  std::unordered_map<std::uint64_t, std::uint64_t> values;
  for (const auto m : program) {
    for (const auto k : generate_values(m.addr | m.mask.to_ullong(), m.float_mask)) {
      values[k] = m.value;
    }
  }
  return ranges::accumulate(
    values | ranges::views::transform([](const auto &v) { return v.second; }),
    0ULL
  );
}

int main(int argc, char **argv)
{
  const auto program = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", part_1(program));
  fmt::print("Part 2: {}\n", part_2(program));
}
