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
#include <stack>

// TODO: Error handling in parsing should not throw
// TODO: count_all_children with general purpose DFS

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

using lookup_graph_t = std::vector<std::vector<std::pair<std::size_t, int>>>;
std::pair<std::unordered_map<std::string, std::size_t>, lookup_graph_t> parse(std::istream &&is)
{
  std::unordered_map<std::string, std::size_t> names;
  lookup_graph_t graph;
  const auto find_index = [&](const std::string& name) {
    if (const auto it = names.find(name); it != names.cend()) {
      return it->second;
    } else {
      auto next = graph.size();
      graph.emplace_back();
      names.emplace(name, next);
      return next;
    }
  };
  for (std::string line; std::getline(is, line);)
  {
    const auto [source, dests] = parse_line(std::move(line));
    const auto parent_index = find_index(source);
    for (const auto& [value, name] : dests) {
      const auto child_index = find_index(name);
      graph[parent_index].emplace_back(child_index, value);
    }
  }
  return std::pair{ std::move(names), std::move(graph) };
}

std::vector<std::vector<std::size_t>> invert(const lookup_graph_t& g)
{
  std::vector<std::vector<std::size_t>> graph(g.size());
  for (std::size_t i = 0; i < g.size(); ++i) {
    for (const auto& [child, count] : g[i]) {
      graph[child].push_back(i);
    }
  }
  return graph;
}

std::size_t count_parents(const std::vector<std::vector<std::size_t>>& graph, std::size_t node)
{
  std::unordered_set<std::size_t> visited;
  std::stack<std::size_t> stack;
  stack.push(node);
  while (!stack.empty()) {
    const auto curr = stack.top();
    stack.pop();
    if (!visited.contains(curr)) {
      visited.insert(curr);
      for (auto part : graph[curr]) {
        stack.push(std::move(part));
      }
    }
  }
  return visited.size() - 1;
}

int count_all_children(const lookup_graph_t &graph, std::size_t node, std::vector<int> memo = {})
{
  if (memo.empty()) {
    memo.resize(graph.size());
    ranges::fill(memo, -1);
  }

  if (const auto value = memo[node]; value >= 0) {
    return value;
  } else if (const auto& children = graph[node]; children.empty()) {
    memo[node] = 0;
    return 0;
  } else {
    int count = 0;
    for (const auto& [child, n] : children) {
      count += n * (count_all_children(graph, child, memo) + 1);
    }
    memo[node] = count;
    return count;
  }
}

int main(int argc, char **argv)
{
  const auto [names, graph] = parse(load_input(argc, argv));
  const auto shiny_gold_index = names.find("shiny gold")->second;
  fmt::print("Part 1: {}\n", count_parents(invert(graph), shiny_gold_index));
  fmt::print("Part 2: {}\n", count_all_children(graph, shiny_gold_index));
}
