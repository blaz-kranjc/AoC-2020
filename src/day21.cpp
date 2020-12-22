#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <vector>
#include <unordered_map>
#include <algorithm>

struct Food
{
  std::vector<std::string> ingredients;// Sorted for set_difference
  std::vector<std::string> alergens;
};

std::vector<std::string> parse_ingredients(std::string_view s)
{
  std::vector<std::string> result;
  auto it = s.begin();
  while (it != s.end()) {
    const auto end = ranges::find(it, s.end(), ' ');
    result.push_back(std::string{ it, end });
    it = end + 1;
  }
  ranges::sort(result);
  return result;
}

std::vector<std::string> parse_alergens(std::string_view s)
{
  if (!s.starts_with("(contains ")) {
    throw std::runtime_error("Alergen list format invalid.");
  }
  std::vector<std::string> result;
  auto it = s.begin() + 10;
  while (true) {
    const auto end = ranges::find_if(it, s.end(), [](auto c) { return c == ',' || c == ')'; });
    result.push_back(std::string{ it, end });
    if (end == s.end() || *end == ')') {
      break;
    } else {
      it = end + 2;
    }
  }
  return result;
}

Food parse_food(std::string_view food)
{
  const auto alergens_begin = ranges::find(food, '(');
  return Food{
    parse_ingredients(std::string_view(food.cbegin(), alergens_begin)),
    parse_alergens(std::string_view(alergens_begin, food.cend()))
  };
}

std::vector<Food> parse(std::istream &&is)
{
  std::vector<Food> result;
  for (std::string line; std::getline(is, line);) {
    result.push_back(parse_food(line));
  }
  return result;
}

std::unordered_map<std::string, std::vector<std::string>> candidates(const std::vector<Food> &foods)
{
  std::unordered_map<std::string, std::vector<std::string>> result;
  for (const auto &f : foods) {
    for (const auto &a : f.alergens) {
      if (auto it = result.find(a); it != result.end()) {
        std::vector<std::string> intersection;
        std::set_intersection(it->second.begin(), it->second.end(), f.ingredients.begin(), f.ingredients.end(), std::back_inserter(intersection));
        it->second = intersection;
      } else {
        result[a] = f.ingredients;
      }
    }
  }
  return result;
}

std::vector<std::string> dangerous_list(std::unordered_map<std::string, std::vector<std::string>> candidates)
{
  std::vector<std::pair<std::string, std::string>> result;
  const auto find_next = [&]() {
    return ranges::find_if(candidates, [](const auto &it) { return it.second.size() == 1; });
  };
  for (auto it = find_next(); it != candidates.end(); it = find_next()) {
    auto alergen = std::move(it->first);
    auto ingredient = std::move(it->second[0]);
    candidates.erase(it);
    for (auto &&[k, vs] : candidates) {
      if (auto it = ranges::find(vs, ingredient); it != vs.end()) {
        vs.erase(it);
      }
    }
    result.push_back(std::pair{ std::move(alergen), std::move(ingredient) });
  }
  ranges::sort(result, [](const auto &l, const auto &r) { return l.first < r.first; });
  return result
         | ranges::views::transform([](auto &&el) { return std::move(el.second); })
         | ranges::to<std::vector>();
}

int main(int argc, char **argv)
{
  const auto foods = parse(load_input(argc, argv));
  auto alergen_candidates = candidates(foods);
  fmt::print(
    "Part 1: {}\n",
    ranges::accumulate(
      foods
        | ranges::views::transform([&](const auto &f) {
            return ranges::count_if(
              f.ingredients,
              [&](const auto &i) {
                return ranges::none_of(alergen_candidates, [&](const auto &p) {
                  return ranges::binary_search(p.second, i);
                });
              });
          }),
      0ll));
  auto alergens = dangerous_list(std::move(alergen_candidates));
  fmt::print(
    "Part 2: {}\n",
    ranges::accumulate(
      alergens | ranges::views::drop(1),
      alergens[0],
      [](auto &&acc, const auto &al) {
        return acc + ',' + al;
      }));
}
