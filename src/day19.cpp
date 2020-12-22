#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <vector>
#include <string>
#include <variant>
#include <unordered_map>
#include <charconv>

template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

int to_int(std::string_view sv) {
  int i;
  std::from_chars(sv.data(), sv.data() + sv.size(), i);
  return i;
}

using Rule = std::vector<int>;

struct OrRule
{
  Rule l;
  Rule r;
};

using Node = std::variant<Rule, OrRule, char>;
using RulesTree = std::unordered_map<int, Node>;

std::pair<int, Node> parse_rule(std::string_view rule_text) {
  auto column = ranges::find(rule_text.begin(), rule_text.end(), ':');
  auto id = to_int(std::string_view{ rule_text.begin(), column });
  if (*(column + 2) == '"') {
    return { id, *(column + 3) };
  }
  Rule l;
  Rule r;
  Rule *rule_ptr = &l;

  auto begin = column + 2;
  while (begin != rule_text.end()) {
    const auto end = ranges::find(begin, rule_text.end(), ' ');
    if (*begin == '|') {
      rule_ptr = &r;
    } else {
      rule_ptr->push_back(to_int(std::string_view(begin, end)));
    }
    if (end == rule_text.end()) {
      break;
    }
    begin = end + 1;
  }

  if (r.empty()) {
    return { id, std::move(l) };
  } else {
    return { id, OrRule{ std::move(l), std::move(r) } };
  }
}

std::pair<RulesTree, std::vector<std::string>> parse(std::istream &&is)
{
  RulesTree rules;
  std::vector<std::string> messages;
  for (std::string line; std::getline(is, line) && !line.empty();) {
    auto [id, rule] = parse_rule(line);
    rules[id] = std::move(rule);
  }
  for (std::string line; std::getline(is, line);) {
    messages.push_back(std::move(line));
  }
  return std::pair{ std::move(rules), std::move(messages) };
}

std::vector<std::string_view> matches_impl(const RulesTree& tree, int node_id, std::string_view message) {
  if (message.empty()) return {};

  const auto &node = tree.find(node_id)->second;
  const auto visit_rule = [&](const Rule &r) {
    return ranges::accumulate(
      r,
      std::vector{ message },
      [&](const auto& msgs, auto id) {
        return msgs
            | ranges::views::transform([&](auto msg) { return matches_impl(tree, id, msg); })
            | ranges::views::cache1
            | ranges::views::join
            | ranges::to<std::vector>;
      });
  };
  const auto visit_or = [&](const OrRule& r) {
    const auto msg_l = visit_rule(r.l);
    const auto msg_r = visit_rule(r.r);
    return ranges::views::concat(msg_l, msg_r) | ranges::to<std::vector>;
  };
  const auto visit_char = [&](char c) {
    if (message[0] == c) {
      return std::vector{ message.substr(1) };
    } else {
      return std::vector<std::string_view>{};
    }
  };
  return std::visit(overloaded{ visit_rule, visit_or, visit_char }, node);
}

bool matches(const RulesTree &tree, std::string_view message)
{
  return ranges::any_of(matches_impl(tree, 0, message), &std::string_view::empty);
}

std::vector<std::string_view> test(std::string_view s) { return { s }; };

int main(int argc, char **argv)
{
  auto [rules, messages] = parse(load_input(argc, argv));
  fmt::print(
    "Part 1: {}\n",
    ranges::count_if(messages, [&](const auto &msg) { return matches(rules, msg); }));

  rules[8] = OrRule{ Rule{ 42 }, Rule{ 42, 8 } };
  rules[11] = OrRule{ Rule{ 42, 31 }, Rule{ 42, 11, 31 } };
  fmt::print(
    "Part 2: {}\n",
    ranges::count_if(messages, [&](const auto &msg) { return matches(rules, msg); }));
}
