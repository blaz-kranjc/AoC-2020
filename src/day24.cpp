#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <array>
#include <set>
#include <map>

using Coord = std::array<int, 3>;
constexpr Coord east{ +1, -1, 0 };
constexpr Coord west{ -1, +1, 0 };
constexpr Coord south_east{ 0, -1, +1 };
constexpr Coord north_west{ 0, +1, -1 };
constexpr Coord north_east{ +1, 0, -1 };
constexpr Coord south_west{ -1, 0, +1 };
constexpr std::array neighbours{ east, west, south_east, south_west, north_east, north_west };

Coord operator+(Coord l, Coord r) {
  return Coord{ l[0] + r[0], l[1] + r[1], l[2] + r[2] };
}

std::vector<Coord> parse_coordinates(std::string_view s) {
  std::vector<Coord> result;
  while (!s.empty()) {
    if (s.starts_with("e")) {
      result.push_back(east);
      s = s.substr(1);
    } else if (s.starts_with("w")) {
      result.push_back(west);
      s = s.substr(1);
    } else if (s.starts_with("sw")) {
      result.push_back(south_west);
      s = s.substr(2);
    } else if (s.starts_with("se")) {
      result.push_back(south_east);
      s = s.substr(2);
    } else if (s.starts_with("ne")) {
      result.push_back(north_east);
      s = s.substr(2);
    } else if (s.starts_with("nw")) {
      result.push_back(north_west);
      s = s.substr(2);
    } else {
      throw std::runtime_error("Unable to parse the coordinates.");
    }
  }
  return result;
}

std::vector<std::vector<Coord>> parse(std::istream&& is) {
  std::vector<std::vector<Coord>> result;
  for (std::string line; std::getline(is, line);) {
    result.push_back(parse_coordinates(line));
  }
  return result;
}

std::set<Coord> get_black(const std::vector<std::vector<Coord>> &coordinates)
{
  std::set<Coord> blacks;
  for (const auto &path : coordinates) {
    Coord end{ 0, 0, 0 };
    for (const auto v : path) {
      end = end + v;
    }
    const auto [it, inserted] = blacks.insert(end);
    if (!inserted) {
      blacks.erase(it);
    }
  }
  return blacks;
}

std::set<Coord> propagate(std::set<Coord> blacks, int count) {
  for (int i = 0; i < count; ++i) {
    std::map<Coord, int> black_neighbours;
    for (const auto p : blacks) {
      for (const auto n : neighbours) {
        black_neighbours[p + n] += 1;
      }
    }
    std::set<Coord> new_blacks;
    for (const auto [p, v] : black_neighbours) {
      const auto is_black = blacks.contains(p);
      if ((is_black && (v == 1 || v == 2)) || (!is_black && v == 2)) {
        new_blacks.insert(p);
      }
    }
    std::swap(new_blacks, blacks);
  }
  return blacks;
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));
  const auto black = get_black(data);
  fmt::print("Part 1: {}\n", black.size());
  fmt::print("Part 2: {}\n", propagate(black, 100).size());
}
