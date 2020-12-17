#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <utility>
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

template<typename... Ts>
struct hash<std::tuple<Ts...>>
{
  constexpr std::size_t operator()(const std::tuple<Ts...> &t) const
  {
    std::size_t hash = 0;
    std::apply([&hash](const auto &... args) { (hash_combine(hash, args), ...); }, t);
    return hash;
  }
};

} // namespace std

using quarternion_t = std::tuple<int, int, int, int>;

std::unordered_set<quarternion_t> parse(std::istream &&is) {
  std::unordered_set<quarternion_t> result;
  int row = 0;
  for (std::string line; std::getline(is, line); ++row) {
    for (int i = 0; i < line.size(); ++i) {
      if (line[i] == '#') {
        result.emplace(row, i, 0, 0);
      }
    }
  }
  return result;
}

constexpr quarternion_t operator+(quarternion_t l, quarternion_t r)
{
  return quarternion_t{ std::get<0>(l) + std::get<0>(r), std::get<1>(l) + std::get<1>(r), std::get<2>(l) + std::get<2>(r), std::get<3>(l) + std::get<3>(r) };
}

static constexpr std::array<quarternion_t, (3 * 3 * 3) - 1> neighbours_3d = []{
  std::array<quarternion_t, (3 * 3 * 3) - 1> result;
  auto it = result.begin();
  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      for (int z = -1; z <= 1; ++z) {
        if (x != 0 || y != 0 || z != 0) {
          *it = quarternion_t{ x, y, z, 0 };
          ++it;
        }
      }
    }
  }
  return result;
}();

static constexpr std::array<quarternion_t, (3 * 3 * 3 * 3) - 1> neighbours_4d = []{
  std::array<quarternion_t, (3 * 3 * 3 * 3) - 1> result;
  auto it = result.begin();
  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      for (int z = -1; z <= 1; ++z) {
        for (int w = -1; w <= 1; ++w) {
          if (x != 0 || y != 0 || z != 0 || w != 0) {
            *it = quarternion_t{ x, y, z, w };
            ++it;
          }
        }
      }
    }
  }
  return result;
}();

template <int N>
std::unordered_set<quarternion_t> step(const std::unordered_set<quarternion_t>& prev, const std::array<quarternion_t, N>& neighbours) {
  std::unordered_map<quarternion_t, int> map;
  for (const auto p : prev) {
    for (const auto n : neighbours) {
      map[n + p] += 1;
    }
  }
  std::unordered_set<quarternion_t> result;
  for (const auto [p, count] : map) {
    if (bool active = prev.contains(p); (active && (count == 2 || count == 3)) || (!active && count == 3)) {
      result.insert(p);
    }
  }
  return result;
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));

  auto prop_3d = data;
  for (int i = 0; i < 6; ++i) {
    prop_3d = step(prop_3d, neighbours_3d);
  }
  fmt::print("Part 1: {}\n", prop_3d.size());

  auto prop_4d = data;
  for (int i = 0; i < 6; ++i) {
    prop_4d = step(prop_4d, neighbours_4d);
  }
  fmt::print("Part 2: {}\n", prop_4d.size());
}
