#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <unordered_map>

std::vector<int> parse(std::istream &&is)
{
  std::vector<int> result;
  for (std::string d; std::getline(is, d, ',');) {
    result.push_back(std::stoi(d));
  }
  return result;
}

int nth_called(const std::vector<int>& start, int n) {
  std::unordered_map<int, int> visited;
  int turn = 1;
  for (const auto c : start | ranges::views::drop_last(1)) {
    visited[c] = turn;
    ++turn;
  }
  int current = start.back();
  for (; turn < n; ++turn) {
    int next = [&] {
      if (const auto it = visited.find(current); it != visited.cend()) {
        return turn - it->second;
      } else {
        return 0;
      }
    }();
    visited[current] = turn;
    current = next;
  }
  return current;
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", nth_called(data, 2020));
  fmt::print("Part 2: {}\n", nth_called(data, 30'000'000));
}
