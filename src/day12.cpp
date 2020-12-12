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

struct Position {
  int x;
  int y;
};

constexpr Position operator+(Position l, Position r) {
  return Position{ l.x + r.x, l.y + r.y };
}

constexpr Position operator*(int c, Position l) {
  return Position{ l.x * c, l.y * c };
}

enum class Direction {
  Left,
  Rigth
};

constexpr Position rotate(Position p, Direction d, int count) {
  for (int i = 0; i < count % 4; ++i) {
    std::swap(p.x, p.y);
    if (d == Direction::Left) {
      p.x *= -1;
    } else {
      p.y *= -1;
    }
  }
  return p;
}

int length(Position p) {
  return std::abs(p.x) + std::abs(p.y);
}

constexpr Position east{ 1, 0 };
constexpr Position north{ 0, 1 };
constexpr Position west{ -1, 0 };
constexpr Position south{ 0, -1 };

Position execute_part_1(const std::vector<Instruction>& is) {
  return ranges::accumulate(
    is,
    std::pair{ Position{ 0, 0 }, east },
    [](auto acc, Instruction i) {
      const auto &[position, direction] = acc;
      if (i.type == 'F') {
        return std::pair{ position + i.amount * direction, direction };
      } else if (i.type == 'R') {
        return std::pair{ position, rotate(direction, Direction::Rigth, i.amount / 90) };
      } else if (i.type == 'L') {
        return std::pair{ position, rotate(direction, Direction::Left, i.amount / 90) };
      } else if (i.type == 'E') {
        return std::pair{ position + i.amount * east, direction };
      } else if (i.type == 'W') {
        return std::pair{ position + i.amount * west, direction };
      } else if (i.type == 'N') {
        return std::pair{ position + i.amount * north, direction };
      } else if (i.type == 'S') {
        return std::pair{ position + i.amount * south, direction };
      }
  }).first;
}

Position execute_part_2(const std::vector<Instruction>& is) {
  return ranges::accumulate(
    is,
    std::pair{ Position{ 0, 0 }, Position{ 10, 1 } },
    [](auto acc, Instruction i) {
      const auto &[position, waypoint] = acc;
    if (i.type == 'F') {
      return std::pair{ position + i.amount * waypoint, waypoint };
    } else if (i.type == 'R') {
      return std::pair{ position, rotate(waypoint, Direction::Rigth, i.amount / 90) };
    } else if (i.type == 'L') {
      return std::pair{ position, rotate(waypoint, Direction::Left, i.amount / 90) };
    } else if (i.type == 'E') {
      return std::pair{ position, waypoint + i.amount * east };
    } else if (i.type == 'W') {
      return std::pair{ position, waypoint + i.amount * west };
    } else if (i.type == 'N') {
      return std::pair{ position, waypoint + i.amount * north };
    } else if (i.type == 'S') {
      return std::pair{ position, waypoint + i.amount * south };
    }
  }).first;
}

int main(int argc, char** argv)
{
  const auto data = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", length(execute_part_1(data)));
  fmt::print("Part 2: {}\n", length(execute_part_2(data)));
}
