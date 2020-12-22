#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <array>
#include <bitset>
#include <vector>
#include <regex>
#include <map>

struct Block
{
  int id;
  std::array<std::array<bool, 10>, 10> picture;
};

std::regex id_line_regex{ R"(Tile (\d+):)"};

Block parse_block(const std::vector<std::string>& block) {
  std::smatch match;
  if (!std::regex_match(block[0], match, id_line_regex)) {
    throw std::runtime_error{ "Missing ID." };
  }
  const auto id = std::stoi(match[1]);
  std::array<std::array<bool, 10>, 10> picture;

  if (block.size() - 1 != 10 || block[1].size() != 10) {
    throw std::runtime_error{ "Unexpected block size." };
  }
  for (int row = 0; row < 10; ++row) {
    for (int column = 0; column < 10; ++column) {
      picture[row][column] = block[row + 1][column] == '#';
    }
  }

  return { id, std::move(picture) };
}

using Edge = std::array<bool, 10>;

Edge left_edge(const Block& block) {
  Edge result;
  for (int i = 0; i < 10; ++i) {
    result[i] = block.picture[i][0];
  }
  return result;
}

Edge right_edge(const Block& block) {
  Edge result;
  for (int i = 0; i < 10; ++i) {
    result[i] = block.picture[i][9];
  }
  return result;
}

Edge top_edge(const Block& block) {
  Edge result;
  for (int i = 0; i < 10; ++i) {
    result[i] = block.picture[0][i];
  }
  return result;
}

Edge bottom_edge(const Block& block) {
  Edge result;
  for (int i = 0; i < 10; ++i) {
    result[i] = block.picture[9][i];
  }
  return result;
}

std::array<Edge, 4> edges(const Block& block) {
  return { left_edge(block), right_edge(block), top_edge(block), bottom_edge(block) };
}

std::array<int, 4> find_corners(const std::vector<Block>& blocks) {
  std::map<Edge, std::vector<int>> edge_map;
  for (const auto block : blocks) {
    for (auto edge : edges(block)) {
      if (auto it = edge_map.find(edge); it != edge_map.end()) {
        it->second.push_back(block.id);
      } else {
        ranges::reverse(edge);
        edge_map[edge].push_back(block.id);
      }
    }
  }
  std::map<int, int> single_edge_count;
  for (const auto& [edge, tiles] : edge_map) {
    if (tiles.size() == 1) {
      ++single_edge_count[tiles[0]];
    }
  }
  std::array<int, 4> result;
  auto it = result.begin();
  for (const auto& [k, v] : single_edge_count) {
    if (v == 2) {
      *it = k;
      ++it;
    }
  }
  return result;
}

std::vector<Block> parse(std::istream&& is)
{
  std::vector<Block> result;
  std::vector<std::string> tmp;
  for (std::string line; std::getline(is, line);) {
    if (line.empty()) {
      result.push_back(parse_block(tmp));
      tmp.clear();
    } else {
      tmp.push_back(line);
    }
  }
  return result;
}

int main(int argc, char **argv)
{
  auto blocks = parse(load_input(argc, argv));
  auto corners = find_corners(blocks);
  fmt::print(
    "Part 1: {}\n",
    ranges::accumulate(corners, 1ll, std::multiplies{}));

  fmt::print(
    "Part 2: {}\n",
    "TODO");
}
