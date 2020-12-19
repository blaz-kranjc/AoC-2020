#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <vector>
#include <stack>
#include <string>

std::vector<std::string> parse(std::istream &&is) {
  std::vector<std::string> result;
  for (std::string line; std::getline(is, line);) {
    result.push_back(std::move(line));
  }
  return result;
}

long long evaluate(std::string_view expression, bool part_2 = false) {
  std::stack<char> operators;
  std::stack<long long> values;
  const auto apply_op = [&](char op) {
    auto a = values.top();
    values.pop();
    auto b = values.top();
    values.pop();
    if (op == '+') {
      values.push(a + b);
    } else /* (op == '*') */ {
      values.push(a * b);
    }
  };
  for (auto it = expression.begin(); it != expression.end(); ++it) {
    if (*it == '(') {
      operators.push(*it);
    } else if (*it == ')') {
      while (operators.top() != '(') {
        apply_op(operators.top());
        operators.pop();
      }
      operators.pop();
    } else if (*it == '+') {
      while (!operators.empty() && operators.top() != '(' && (!part_2 || operators.top() != '*')) {
        apply_op(operators.top());
        operators.pop();
      }
      operators.push(*it);
    } else if (*it == '*') {
      while (!operators.empty() && operators.top() != '(') {
        apply_op(operators.top());
        operators.pop();
      }
      operators.push(*it);
    } else if (*it >= '0' && *it <= '9') {
      values.push(*it - '0');
    }
  }
  while (!operators.empty()) {
    apply_op(operators.top());
    operators.pop();
  }
  return values.top();
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));
  auto part_1 = ranges::accumulate(
    data | ranges::views::transform([](const auto &ex) { return evaluate(ex, false); }),
    0ll
  );
  fmt::print("Part 1: {}\n", part_1);
  auto part_2 = ranges::accumulate(
    data | ranges::views::transform([](const auto &ex) { return evaluate(ex, true); }),
    0ll
  );
  fmt::print("Part 2: {}\n", part_2);
}
