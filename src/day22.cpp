#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <deque>
#include <set>
#include <utility>

std::pair<std::deque<int>, std::deque<int>> parse(std::istream&& is) {
  std::deque<int> first;
  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for (std::string line; std::getline(is, line);) {
    if (line.empty()) {
      break;
    } else {
      first.push_back(std::stoi(line));
    }
  }

  std::deque<int> second;
  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for (std::string line; std::getline(is, line);) {
    second.push_back(std::stoi(line));
  }
  return std::pair{ std::move(first), std::move(second) };
}

std::deque<int> play_out_combat(std::deque<int> l, std::deque<int> r) {
  while (!l.empty() && !r.empty()) {
    const auto l_val = l.front();
    const auto r_val = r.front();
    l.pop_front();
    r.pop_front();
    
    if (l_val > r_val) {
      l.push_back(l_val);
      l.push_back(r_val);
    } else {
      r.push_back(r_val);
      r.push_back(l_val);
    }
  }
  return l.empty() ? std::move(r) : std::move(l);
}

int score(std::deque<int> l) {
  return ranges::accumulate(
    l
      | ranges::views::reverse
      | ranges::views::enumerate
      | ranges::views::transform([](auto el) { return static_cast<int>(el.first + 1) * el.second; }),
    0
  );
}

enum class Player {
  One,
  Two
};

std::pair<Player, std::deque<int>> play_out_recursive_combat(std::deque<int> l, std::deque<int> r) {
  std::set<std::pair<std::deque<int>, std::deque<int>>> history;
  while (!l.empty() && !r.empty()) {
    if (const auto [it, is_new] = history.insert(std::pair{ l, r }); !is_new) {
      return std::pair{ Player::One, std::move(l) };
    }
    const auto l_val = l.front();
    const auto r_val = r.front();
    l.pop_front();
    r.pop_front();
    const auto winner = [&] {
      if (l_val > l.size() || r_val > r.size()) {
        return (l_val > r_val) ? Player::One : Player::Two;
      } else {
        return play_out_recursive_combat(
            l | ranges::views::take(l_val) | ranges::to<std::deque>,
            r | ranges::views::take(r_val) | ranges::to<std::deque>
        ).first;
      }
    }();
    
    if (winner == Player::One) {
      l.push_back(l_val);
      l.push_back(r_val);
    } else {
      r.push_back(r_val);
      r.push_back(l_val);
    }
  }
  return l.empty() ? std::pair{ Player::Two, std::move(r) } : std::pair{ Player::One, std::move(l) };
}

int main(int argc, char **argv)
{
  const auto [fst, snd] = parse(load_input(argc, argv));
  fmt::print(
    "Part 1: {}\n",
    score(play_out_combat(fst, snd)));
  fmt::print(
    "Part 2: {}\n",
    score(play_out_recursive_combat(fst, snd).second));
}
