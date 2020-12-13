#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <optional>
#include <vector>
#include <charconv>
#include <numeric>
#include <concepts>

std::optional<int> to_int(std::string_view str)
{
  int result;
  const auto [it, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
  if (ec == std::errc{}) {
    return result;
  } else {
    return std::nullopt;
  }
}

std::pair<int, std::vector<std::optional<int>>> parse(std::istream &&is)
{
  int time;
  is >> time;
  is.ignore();
  std::vector<std::optional<int>> result;
  for (std::string d; std::getline(is, d, ',');) {
    if (d == "x") {
      result.push_back(std::nullopt);
    } else if (const auto val = to_int(d); val) {
      result.push_back(*val);
    }
  }
  return std::pair{ time, result };
}

std::pair<int, int> first_bus(int time, const std::vector<std::optional<int>> &data)
{
  auto wait_times = data
                    | ranges::views::filter(&std::optional<int>::has_value)
                    | ranges::views::transform([time](auto bus) { return std::pair{ *bus, *bus - time % *bus }; });
  return *ranges::min_element(wait_times, [](auto l, auto r) { return l.second < r.second; });
}

struct Equation
{
  int reminder;
  int quotient;
};

int modulo(int n, int q)
{
  if (n > 0) {
    return n % q;
  } else {
    return (n % q) + q;
  }
}

std::vector<Equation> extract_crt(const std::vector<std::optional<int>> &buses)
{
  return buses
         | ranges::views::enumerate
         | ranges::views::filter([](auto p) { return p.second.has_value(); })
         | ranges::views::transform(
           [](auto p) {
             return Equation{ modulo(*p.second - static_cast<int>(p.first), *p.second), *p.second };
           })
         | ranges::to<std::vector>;
}

template<std::integral Int>
std::optional<Int> mod_inverse(Int a, Int n)
{
  std::pair ts{ Int{ 0 }, Int{ 1 } };
  std::pair rs{ n, a };
  while (rs.second != 0) {
    auto q = rs.first / rs.second;
    ts = { ts.second, ts.first - q * ts.second };
    rs = { rs.second, rs.first - q * rs.second };
  }

  if (rs.first > 1) {
    return std::nullopt;
  } else if (ts.first < 0) {
    return ts.first + n;
  } else {
    return ts.first;
  }
}

long long solve_crt(const std::vector<Equation> &equations)
{
  const auto prod = ranges::accumulate(
    equations | ranges::views::transform(&Equation::quotient),
    1ll,
    std::multiplies{});
  const auto prods = equations
                     | ranges::views::transform(
                       [prod](auto eq) {
                         const auto prod_i = prod / eq.quotient;
                         const auto x_i = mod_inverse(prod_i, static_cast<long long>(eq.quotient));
                         if (!x_i) throw std::runtime_error{ "The product is not invertible." };
                         return *x_i * prod_i * eq.reminder;
                       });
  return ranges::accumulate(prods, 0ll) % prod;
}

int main(int argc, char **argv)
{
  const auto [timestamp, buses] = parse(load_input(argc, argv));
  const auto [bus, wait_time] = first_bus(timestamp, buses);
  fmt::print("Part 1: {}\n", bus * wait_time);
  const auto equations = extract_crt(buses);
  fmt::print("Part 2: {}\n", solve_crt(equations));
}
