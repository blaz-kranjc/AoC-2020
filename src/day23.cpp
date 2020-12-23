#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <list>

std::vector<int> parse(std::istream&& is) {
  std::vector<int> result;
  char c;
  while (is >> c) {
    result.push_back(c - '0');
  }
  return result;
}

struct next_list
{
  int head;
  std::vector<int> list;
};

next_list to_next_list(const std::vector<int>& cups) {
  std::vector<int> list(cups.size());
  for (int i = 0; i < cups.size() - 1; ++i) {
    list[cups[i] - 1] = cups[i + 1] - 1;
  }
  list[cups.back() - 1] = cups.front() - 1;
  return next_list{ cups.front() - 1, std::move(list) };
}

int next(const next_list& list, int head) {
  return list.list[head];
}

int next(const next_list& list) {
  return list.list[list.head];
}

int next(const next_list& list, int head, int count) {
  for (; count > 0; --count) {
    head = list.list[head];
  }
  return head;
}

std::vector<int> as_vector(const next_list &cups, int head) {
  std::vector<int> result(cups.list.size());
  int next = head;
  for (int i = 0; i < cups.list.size(); ++i) {
    result[i] = next + 1;
    next = cups.list[next];
  }
  return result;
}

std::vector<int> as_vector(const next_list& cups) {
  return as_vector(cups, cups.head);
}

next_list play_cups(next_list cups, int count) {
  for (int i = 0; i < count; ++i) {
    const auto splice_first = next(cups);
    const auto splice_last = next(cups, splice_first, 2);
    const auto in_splice = [&](int val) {
      return val == splice_first || val == next(cups, splice_first) || val == splice_last;
    };
    const auto insertion = [&] {
      const auto decrement = [&](auto i) { return (i > 0 ? i : cups.list.size()) - 1; };
      auto value = decrement(cups.head);
      for (; in_splice(value); value = decrement(value)) {}
      return value;
    }();
    cups.list[cups.head] = next(cups, splice_last);
    const auto tail = cups.list[insertion];
    cups.list[insertion] = splice_first;
    cups.list[splice_last] = tail;
    cups.head = next(cups);
  }
  return cups;
}

int main(int argc, char **argv)
{
  const auto cups = parse(load_input(argc, argv));
  auto play1 = play_cups(to_next_list(cups), 100);
  auto part1 = as_vector(play1, 0);
  fmt::print(
    "Part 1: {}\n",
    part1
      | ranges::views::drop(1)
      | ranges::views::transform([](auto el) { return el + '0'; })
      | ranges::to<std::string>);
  auto cups2 = cups;
  cups2.reserve(1'000'000);
  while (cups2.size() < 1'000'000) {
    cups2.push_back(cups2.size() + 1);
  }
  auto play2 = play_cups(to_next_list(cups2), 10'000'000);
  long long v1 = next(play2, 0) + 1;
  long long v2 = next(play2, 0, 2) + 1;
  fmt::print("Part 2: {}\n", v1 * v2);
}
