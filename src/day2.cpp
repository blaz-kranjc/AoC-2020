#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <sstream>
#include <string>
#include <tuple>

std::vector<std::tuple<int, int, char, std::string>> parse(std::istream&& is)
{
  std::vector<std::tuple<int, int, char, std::string>> result;

  int min;
  int max;
  char c;
  std::string s;
  for (std::string line; std::getline(is, line);) {
    std::stringstream stream{ line };
    stream >> min;
    stream.ignore();
    stream >> max;
    stream.ignore();
    stream >> c;
    stream.ignore(2);
    stream >> s;
    result.push_back(std::tuple(min, max, c, s));
  }

  return result;
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));

  fmt::print(
    "Part 1: {}\n",
    ranges::count_if(data, [](const auto &el) {
      const auto &[min, max, c, s] = el;
      const auto n_chars = ranges::count(s, c);
      return n_chars >= min && n_chars <= max;
    }));

  fmt::print(
    "Part 2: {}\n",
    ranges::count_if(data, [](const auto &el) {
      const auto &[min, max, c, s] = el;
      return (s[min - 1] == c) != (s[max - 1] == c);
    }));
}
