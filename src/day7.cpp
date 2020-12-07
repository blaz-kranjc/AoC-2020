#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <utility>
#include <string>
#include <regex>
#include <exception>

std::pair<std::string, std::vector<std::pair<int, std::string>>> parse_line(std::string line)
{
  static const std::regex definition_base{ "([a-z]+ [a-z]+) bags contain " };
  static const std::regex containing_definition{ "([0-9]+) ([a-z]+ [a-z]+) bag" };

  if (std::smatch definition_match; std::regex_search(line, definition_match, definition_base)) {
    const auto name = definition_match[1];
    auto iter = std::sregex_iterator(line.begin(), line.end(), containing_definition);
    std::vector<std::pair<int, std::string>> contained;
    for (; iter != std::sregex_iterator{}; ++iter) {
      contained.emplace_back(std::stoi((*iter)[1].str()), (*iter)[2]);
    }
    return { name, std::move(contained) };
  } else {
    throw std::runtime_error{ "The line does not match the bags." };
  }
}

using lookup_graph_t = std::unordered_map<std::string, std::vector<std::pair<int, std::string>>>;
lookup_graph_t parse(std::istream &&is)
{
  lookup_graph_t map{};
  for (std::string line; std::getline(is, line);) {
    map.insert(parse_line(std::move(line)));
  }
  return map;
}

std::unordered_map<std::string, std::vector<std::string>> invert(const lookup_graph_t& g)
{
  std::unordered_map<std::string, std::vector<std::string>> graph;
  for (const auto& node : g) {
    for (const auto& [count, child] : node.second) {
      graph[child].push_back(node.first);
    }
  }
  return graph;
}

int count_parents(const std::unordered_map<std::string, std::vector<std::string>>& parent_graph, const std::string& node)
{
  std::unordered_set<std::string> visited;
  std::vector<std::string> stack{ node };
  while (!stack.empty()) {
    const auto curr = stack.back();
    stack.pop_back();
    if (!visited.contains(curr)) {
      visited.insert(curr);
      if (const auto it = parent_graph.find(curr); it != parent_graph.end()) {
        const auto &parts = it->second;
        stack.insert(stack.end(), parts.begin(), parts.end());
      }
    }
  }
  return visited.size() - 1; // Remove the gold bag
}

int count_all_children(const lookup_graph_t& graph, const std::string& node)
{
  const auto it = graph.find(node);
  if (it == graph.end() || it->second.empty()) {
    return 1;
  } else {
    int count = 1;
    for (const auto& [n, child] : it->second) {
      count += n * count_all_children(graph, child);
    }
    return count;
  }
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", count_parents(invert(data), "shiny gold"));
  fmt::print("Part 1: {}\n", count_all_children(data, "shiny gold") - 1);
}
