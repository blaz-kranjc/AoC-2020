#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <string>
#include <charconv>
#include <cctype>

template<int min, int max>
bool is_number_between(std::string_view data)
{
  auto value{ 0 };
  const auto [ptr, ec] = std::from_chars(data.data(), data.data() + data.size(), value);
  return (ptr == data.data() + data.size()) && value <= max && value >= min;
}

bool is_valid_hex(std::string_view data)
{
  return data.size() == 7
         && data[0] == '#'
         && ranges::all_of(
           data | ranges::views::drop(1),
           [](auto c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); });
}

constexpr bool non_empty(std::string_view data)
{
  return !data.empty();
}

template<auto c>
constexpr auto constant(auto) { return c; }

static constexpr std::array valid_colors{ "amb", "blu", "brn", "gry", "grn", "hzl", "oth" };

using validator_t = bool(*)(std::string_view);
using field_t = std::tuple<std::string_view, validator_t, validator_t>;
static constexpr std::array fields{
  field_t{ "byr", non_empty, is_number_between<1920, 2002> },
  field_t{ "iyr", non_empty, is_number_between<2010, 2020> },
  field_t{ "eyr", non_empty, is_number_between<2020, 2030> },
  field_t{
    "hgt",
    non_empty,
    [](std::string_view data) {
          if (const auto measurement = data.substr(0, data.size() - 2); data.ends_with("cm")) {
          return is_number_between<150, 193>(measurement);
        } else if (data.ends_with("in")) {
          return is_number_between<59, 76>(measurement);
        } else {
          return false;
        } } },
  field_t{ "hcl", non_empty, is_valid_hex },
  field_t{
    "ecl",
    non_empty,
    [](std::string_view data) { return ranges::contains(valid_colors, data); } },
  field_t{
    "pid",
    non_empty,
    [](std::string_view data) {
      return data.size() == 9 && ranges::all_of(data, [](auto c) { return std::isdigit(c); });
    } },
  field_t{ "cid", constant<true>, constant<true> },
};

std::string_view tag(std::string_view field) { return field.substr(0, 3); }
std::string_view value(std::string_view field) { return field.substr(4); }

using passport_t = std::array<std::string, fields.size()>;

std::vector<passport_t> parse(std::istream &&is)
{
  std::vector<passport_t> result;

  std::string current_field;
  passport_t current_passport;
  bool has_only_valid_tags = true;
  while (!is.eof()) {
    is >> current_field;
    const auto tag_it = ranges::find_if(fields, [t = tag(current_field)](const auto &e) { return std::get<0>(e) == t; });
    if (tag_it == fields.end()) {
      has_only_valid_tags= false;
    } else if (const auto ind = std::distance(fields.cbegin(), tag_it); current_passport[ind].empty()) {
      current_passport[ind] = value(current_field);
    } else {
      has_only_valid_tags = false;
    }

    is.ignore();
    if (is.peek() == '\n') {
      is.ignore();
      if (has_only_valid_tags) {
        result.push_back(std::move(current_passport));
      }
      current_passport.fill("");
      has_only_valid_tags = true;
    }
  }
  result.push_back(std::move(current_passport));

  return result;
}

template <int validator_index>
bool is_valid(const passport_t &data)
{
  return ranges::all_of(
    ranges::views::zip(fields, data),
    [](const auto &el) { return std::get<validator_index>(el.first)(el.second); });
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", ranges::count_if(data, is_valid<1>));
  fmt::print("Part 2: {}\n", ranges::count_if(data, is_valid<2>));
}
