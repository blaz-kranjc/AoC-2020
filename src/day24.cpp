#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <array>
#include <unordered_set>
#include <unordered_map>

template<typename T>
inline constexpr void hash_combine(std::size_t &s, const T &v)
{
  // From boost::hash_combine
  s ^= std::hash<T>()(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
};


namespace std {

template<typename T, std::size_t N>
struct hash<std::array<T, N>>
{
  constexpr std::size_t operator()(const std::array<T, N> &v) const
  {
    std::size_t hash = 0;
    for (const auto &el : v) {
      hash_combine(hash, el);
    }
    return hash;
  }
};

} // namespace std

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

Coord parse_coordinate(std::string_view s) {
  Coord result{ 0, 0, 0 };
  while (!s.empty()) {
    if (s.starts_with("e")) {
      result = result + east;
      s = s.substr(1);
    } else if (s.starts_with("w")) {
      result = result + west;
      s = s.substr(1);
    } else if (s.starts_with("sw")) {
      result = result + south_west;
      s = s.substr(2);
    } else if (s.starts_with("se")) {
      result = result + south_east;
      s = s.substr(2);
    } else if (s.starts_with("ne")) {
      result = result + north_east;
      s = s.substr(2);
    } else if (s.starts_with("nw")) {
      result = result + north_west;
      s = s.substr(2);
    } else {
      throw std::runtime_error("Unable to parse the coordinates.");
    }
  }
  return result;
}

std::unordered_set<Coord> parse(std::istream&& is) {
  std::unordered_set<Coord> result;
  for (std::string line; std::getline(is, line);) {
    const auto coord = parse_coordinate(line);
    const auto [it, inserted] = result.insert(coord);
    if (!inserted) {
      result.erase(it);
    }
  }
  return result;
}

std::unordered_set<Coord> propagate(std::unordered_set<Coord> blacks, int count) {
  for (int i = 0; i < count; ++i) {
    std::unordered_map<Coord, int> black_neighbours;
    for (const auto p : blacks) {
      for (const auto n : neighbours) {
        black_neighbours[p + n] += 1;
      }
    }
    std::unordered_set<Coord> new_blacks;
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
  auto data = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", data.size());
  fmt::print("Part 2: {}\n", propagate(std::move(data), 100).size());
}
