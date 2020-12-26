#include "input_file_loader.h"
#include "padded_vector_2d.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <array>
#include <vector>
#include <regex>
#include <map>
#include <stack>

using Picture = std::array<std::array<bool, 8>, 8>;
using Edge = std::array<bool, 10>;

enum class Side {
  Top = 0,
  Right = 1,
  Bottom = 2,
  Left = 3
};
constexpr std::array offsets{
  std::pair{ -1, 0 },
  std::pair{ 0, 1 },
  std::pair{ 1, 0 },
  std::pair{ 0, -1 }
};
constexpr std::array opposite_side{
  static_cast<int>(Side::Bottom),
  static_cast<int>(Side::Left),
  static_cast<int>(Side::Top),
  static_cast<int>(Side::Right)
};

struct Block
{
  int id{ 0 };
  Picture picture;
  std::array<Edge, 4> edges;
};

bool operator<(const Block &l, const Block &r) { return l.id < r.id; }
bool operator==(const Block& l, const Block& r) { return l.id == r.id; }

std::regex id_line_regex{ R"(Tile (\d+):)"};

Block parse_block(const std::vector<std::string>& block) {
  std::smatch match;
  if (!std::regex_match(block[0], match, id_line_regex)) {
    throw std::runtime_error{ "Missing ID." };
  }
  const auto id = std::stoi(match[1]);
  Block result{ id };

  if (block.size() - 1 != 10 || block[1].size() != 10) {
    throw std::runtime_error{ "Unexpected block size." };
  }
  for (int i = 0; i < 10; ++i) {
    result.edges[static_cast<int>(Side::Top)][i] = block[1][i] == '#';
    result.edges[static_cast<int>(Side::Bottom)][i] = block[10][9 - i] == '#';
    result.edges[static_cast<int>(Side::Right)][i] = block[i + 1][9] == '#';
    result.edges[static_cast<int>(Side::Left)][i] = block[9 - i + 1][0] == '#';
  }
  for (int row = 0; row < 8; ++row) {
    for (int column = 0; column < 8; ++column) {
      result.picture[row][column] = block[row + 2][column + 1] == '#';
    }
  }

  return result;
}

std::map<int, Block> parse(std::istream&& is)
{
  std::map<int, Block> result;
  std::vector<std::string> tmp;
  for (std::string line; std::getline(is, line);) {
    if (line.empty()) {
      auto block = parse_block(tmp);
      const auto id = block.id;
      result.emplace(id, std::move(block));
      tmp.clear();
    } else {
      tmp.push_back(line);
    }
  }
  return result;
}

template <typename T, std::size_t N>
constexpr std::array<std::array<T, N>, N> flip_x(std::array<std::array<T, N>, N> m) {
  for (auto row = 0; row < N; ++row) {
    for (auto col = 0; col < N / 2; ++col) {
      std::swap(m[row][col], m[row][N - 1 - col]);
    }
  }
  return m;
}

template <typename T, std::size_t N>
constexpr std::array<std::array<T, N>, N> rotate_right(std::array<std::array<T, N>, N> m) {
  for (int i = 0; i < N / 2; ++i) {
    const auto size = N - 2 * i;
    const auto max = N - 1 - i;
    for (int j = 0; j < size - 1; ++j) {
      std::swap(m[i][j + i], m[j + i][max]);
      std::swap(m[i][j + i], m[max][max - j]);
      std::swap(m[i][j + i], m[max - j][i]);
    }
  }
  return m;
}

void flip_x(Block& b) {
  std::swap(b.edges[static_cast<int>(Side::Left)], b.edges[static_cast<int>(Side::Right)]);
  ranges::for_each(b.edges, ranges::reverse);
  b.picture = flip_x(std::move(b.picture));
}

void rotate_right(Block& b) {
  std::swap(b.edges[static_cast<int>(Side::Top)], b.edges[static_cast<int>(Side::Right)]);
  std::swap(b.edges[static_cast<int>(Side::Top)], b.edges[static_cast<int>(Side::Bottom)]);
  std::swap(b.edges[static_cast<int>(Side::Top)], b.edges[static_cast<int>(Side::Left)]);
  b.picture = rotate_right(std::move(b.picture));
}

std::map<Edge, std::vector<int>> edge_map(const std::map<int, Block>& blocks) {
  std::map<Edge, std::vector<int>> result;
  for (const auto& [id, block] : blocks) {
    for (auto edge : block.edges) {
      // TODO reduce the size of this map by not storing both orientations
      result[edge].push_back(block.id);
      ranges::reverse(edge);
      result[edge].push_back(block.id);
    }
  }
  return result;
}

void orient_to_place(const Block& block, Block& other, int edge) {
  const auto fits = [&]() { return ranges::equal(block.edges[edge], other.edges[opposite_side[edge]] | ranges::views::reverse); };
  if (fits()) return;
  rotate_right(other);
  if (fits()) return;
  rotate_right(other);
  if (fits()) return;
  rotate_right(other);
  if (fits()) return;

  rotate_right(other);
  flip_x(other);
  if (fits()) return;
  rotate_right(other);
  if (fits()) return;
  rotate_right(other);
  if (fits()) return;
  rotate_right(other);
  if (fits()) return;

  throw std::runtime_error("Unable to orient the pieces together");
}

struct Reconstruction
{
  int grid_row_min{ 0 };
  int grid_row_max{ 0 };
  int grid_col_min{ 0 };
  int grid_col_max{ 0 };
  std::map<std::pair<int, int>, Block> inserted;
};

std::vector<std::reference_wrapper<const Block>> corners(const Reconstruction& rec) {
  return {
    rec.inserted.find(std::pair{ rec.grid_row_min, rec.grid_col_min })->second,
    rec.inserted.find(std::pair{ rec.grid_row_min, rec.grid_col_max })->second,
    rec.inserted.find(std::pair{ rec.grid_row_max, rec.grid_col_min })->second,
    rec.inserted.find(std::pair{ rec.grid_row_max, rec.grid_col_max })->second,
  };
}

// TODO: this only handles the case with 1 possible neighbor
Reconstruction reconstruct(std::map<int, Block> blocks) {
  if (blocks.empty()) {
    return {};
  }

  const auto edges = edge_map(blocks);
  
  int grid_row_min{ 0 };
  int grid_row_max{ 0 };
  int grid_column_min{ 0 };
  int grid_column_max{ 0 };
  std::map<std::pair<int, int>, Block> inserted; /* TODO here we store the blocks twice */

  inserted.emplace(std::pair{ 0, 0 }, blocks.begin()->second);
  std::stack<std::pair<int, int>> future;
  future.emplace(std::pair{ 0, 0 });
  while (!future.empty()) {
    const auto [row, column] = future.top();
    future.pop();
    const auto &block = inserted[std::pair{ row, column }];
    for (int i = 0; i < block.edges.size(); ++i) {
      const auto [row_off, col_off] = offsets[i];
      const auto pos = std::pair{ row + row_off, column + col_off };
      if (inserted.contains(pos)) { continue; /* Piece already in place */ }
      const auto &shared_edges = edges.find(block.edges[i])->second;
      if (shared_edges.size() == 1) { continue; /* This is a side piece */ }
      if (shared_edges.size() > 2) { throw std::runtime_error("Cannot reconstruct data with multiple edge candidates."); }

      grid_row_min = std::min(grid_row_min, pos.first);
      grid_row_max = std::max(grid_row_max, pos.first);
      grid_column_min = std::min(grid_column_min, pos.second);
      grid_column_max = std::max(grid_column_max, pos.second);
      const auto other_id = shared_edges[0] == block.id ? shared_edges[1] : shared_edges[0];
      auto other_block = blocks[other_id];
      orient_to_place(block, other_block, i);
      inserted.emplace(pos, std::move(other_block));
      future.push(pos);
    }
  }

  return { .grid_row_min = grid_row_min, .grid_row_max = grid_row_max, .grid_col_min = grid_column_min, .grid_col_max = grid_column_max, .inserted = std::move(inserted) };
}

PaddedVector2D<std::int8_t> picture(const Reconstruction& rec) {
  const std::size_t rows = (rec.grid_row_max - rec.grid_row_min + 1) * 8;
  const std::size_t columns = (rec.grid_col_max - rec.grid_col_min + 1) * 8;
  std::vector<std::int8_t> raw(rows * columns, std::int8_t{ 0 });
  for (int row = rec.grid_row_min; row <= rec.grid_row_max; ++row) {
    for (int col = rec.grid_col_min; col <= rec.grid_col_max; ++col) {
      const auto &pic = rec.inserted.find(std::pair(row, col))->second.picture;
      for (int pic_row = 0; pic_row < pic.size(); ++pic_row) {
        for (int pic_col = 0; pic_col < pic[0].size(); ++pic_col) {
          std::size_t index = ((row - rec.grid_row_min) * 8 + pic_row) * columns + (col - rec.grid_col_min) * 8 + pic_col;
          raw[index] = pic[pic_row][pic_col] ? 1 : 0;
        }
      }
    }
  }

  return PaddedVector2D{ columns, std::int8_t{ 0 }, raw };
}

constexpr std::string_view monster_str =
  "                  # \n"
  "#    ##    ##    ###\n"
  " #  #  #  #  #  #   ";
constexpr auto n_monster_points = [] {
  int count = 0;
  for (const auto c : monster_str) {
    if (c == '#') ++count;
  }
  return count;
}();
constexpr auto original_monster_positions = [] {
  std::array<std::pair<int, int>, n_monster_points> result;
  auto it = result.begin();
  int column = 0;
  int row = 0;
  for (const auto c : monster_str) {
    if (c == '\n') {
      column = 0;
      ++row;
    } else {
      if (c == '#') {
        *it = std::pair{ row, column };
        ++it;
      }
      ++column;
    }
  }
  return result;
}();
constexpr auto max_y = [](const auto& poss) {
  return std::max_element(poss.begin(), poss.end(), [](auto l, auto r) { return l.first < r.first; })->first;
};
constexpr auto max_x = [](const auto &poss) {
  return std::max_element(poss.begin(), poss.end(), [](auto l, auto r) { return l.second < r.second; })->second;
};

constexpr std::array<std::pair<int, int>, n_monster_points> flip_monster(std::array<std::pair<int, int>, n_monster_points> monster) {
  const auto grid_row_max = max_x(monster);
  std::transform(monster.begin(), monster.end(), monster.begin(), [grid_row_max](auto pos) { return std::pair{ pos.first, grid_row_max - pos.second }; });
  return monster;
}

constexpr std::array<std::pair<int, int>, n_monster_points> rotate_monster(std::array<std::pair<int, int>, n_monster_points> monster) {
  const auto grid_row_max = max_x(monster);
  const auto grid_col_max = max_y(monster);
  std::transform(
    monster.begin(),
    monster.end(),
    monster.begin(),
    [=](auto pos) {
      return std::pair{ -1 * pos.second + grid_row_max, pos.first };
    });
  return monster;
}

constexpr std::array monsters{
  original_monster_positions,
  rotate_monster(original_monster_positions),
  rotate_monster(rotate_monster(original_monster_positions)),
  rotate_monster(rotate_monster(rotate_monster(original_monster_positions))),
  flip_monster(original_monster_positions),
  rotate_monster(flip_monster(original_monster_positions)),
  rotate_monster(rotate_monster(flip_monster(original_monster_positions))),
  rotate_monster(rotate_monster(rotate_monster(flip_monster(original_monster_positions))))
};

int find_monsters(const PaddedVector2D<std::int8_t>& pic, const std::array<std::pair<int, int>, n_monster_points>& monster) {
  int count = 0;
  for (int r = 0; r < pic.rows(); ++r) {
    for (int c = 0; c < pic.cols(); ++c) {
      if (ranges::all_of(
            monster | ranges::views::transform([r, c](auto p) { return std::pair{ p.first + r, p.second + c }; }),
            [&](auto p) { return pic.at(p.first, p.second) == 1; })) {
        ++count;
      }
    }
  }
  return count;
}

int find_monsters(const PaddedVector2D<std::int8_t>& pic) {
  for (const auto& m : monsters) {
    auto count = find_monsters(pic, m);
    if (count > 0) {
      return count;
    }
  }
  return 0;
}

int main(int argc, char **argv)
{
  auto blocks = parse(load_input(argc, argv));
  const auto rec = reconstruct(std::move(blocks));
  auto prod = 1ll;
  for (const auto& b : corners(rec)) {
    prod *= b.get().id;
  }
  fmt::print("Part 1: {}\n", prod);
  const auto pic = picture(rec);
  const auto all_occupied = ranges::count(pic.raw(), 1);
  fmt::print(
    "Part 2: {}\n",
    all_occupied - find_monsters(pic) * n_monster_points);
}
