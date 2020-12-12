#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <vector>
#include <array>
#include <charconv>

struct Instruction
{
  char type;
  int amount;
};

std::vector<Instruction> parse(std::istream&& is) {
  std::vector<Instruction> result;
  for (std::string line; std::getline(is, line);) {
    int n;
    std::from_chars(line.data() + 1, line.data() + line.size(), n);
    result.emplace_back(line[0], n);
  }
  return result;
}

constexpr std::array directions{ std::pair{ 1, 0 }, std::pair{ 0, 1 }, std::pair{ -1, 0 }, std::pair{ 0, -1 } };
constexpr auto east_id = 0;
constexpr auto north_id = 1;
constexpr auto west_id = 2;
constexpr auto south_id = 3;

std::pair<int, int> execute(const std::vector<Instruction>& is) {
  std::pair position{ 0, 0 };
  int direction = east_id;
  for (auto i : is) {
    if (i.type == 'F') {
      position = std::pair{ position.first + directions[direction].first * i.amount, position.second + directions[direction].second * i.amount };
    } else if (i.type == 'R') {
      direction = (direction - (i.amount / 90) + directions.size()) % directions.size();
    } else if (i.type == 'L') {
      direction = (direction + i.amount / 90) % directions.size();
    } else if (i.type == 'E') {
      position = std::pair{ position.first + directions[east_id].first * i.amount, position.second + directions[east_id].second * i.amount };
    } else if (i.type == 'W') {
      position = std::pair{ position.first + directions[west_id].first * i.amount, position.second + directions[west_id].second * i.amount };
    } else if (i.type == 'N') {
      position = std::pair{ position.first + directions[north_id].first * i.amount, position.second + directions[north_id].second * i.amount };
    } else if (i.type == 'S') {
      position = std::pair{ position.first + directions[south_id].first * i.amount, position.second + directions[south_id].second * i.amount };
    }
  }
  return position;
}

std::pair<int, int> execute2(const std::vector<Instruction>& is) {
  std::pair position{ 0, 0 };
  std::pair waypoint{ 10, 1 };
  for (auto i : is) {
    if (i.type == 'F') {
      position.first += i.amount * waypoint.first;
      position.second += i.amount * waypoint.second;
    } else if (i.type == 'R') {
      for (int r = 0; r < i.amount / 90; ++r) {
        std::swap(waypoint.first, waypoint.second);
        waypoint.second *= -1;
      }
    } else if (i.type == 'L') {
      for (int r = 0; r < i.amount / 90; ++r) {
        std::swap(waypoint.first, waypoint.second);
        waypoint.first *= -1;
      }
    } else if (i.type == 'E') {
      waypoint.first += i.amount;
    } else if (i.type == 'W') {
      waypoint.first -= i.amount;
    } else if (i.type == 'N') {
      waypoint.second += i.amount;
    } else if (i.type == 'S') {
      waypoint.second -= i.amount;
    }
  }
  return position;
}

int main(int argc, char** argv)
{
  const auto data = parse(load_input(argc, argv));
  const auto end_pos = execute(data);
  fmt::print("Part 1: {}\n", std::abs(end_pos.first) + std::abs(end_pos.second));
  const auto end_pos_2 = execute2(data);
  fmt::print("Part 2: {}\n", std::abs(end_pos_2.first) + std::abs(end_pos_2.second));
}
