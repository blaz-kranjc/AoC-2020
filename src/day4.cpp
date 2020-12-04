#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <string>
#include <functional>
#include <charconv>

bool is_number_between(std::string_view data, std::pair<int, int> limits)
{
  auto value{ 0 };
  const auto [ptr, ec] = std::from_chars(data.data(), data.data() + data.size(), value);
  return (ptr == data.data() + data.size())
      && (ec == std::errc{})
      && value <= limits.second
      && value >= limits.first;
}

bool is_valid_hex(std::string_view data)
{
  return data.size() == 7
      && data[0] == '#'
      && ranges::all_of(
          data | ranges::views::drop(1),
          [](auto c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }
  );
}

static constexpr std::array tags{ "byr", "iyr", "eyr", "hgt", "hcl", "ecl", "pid", "cid" };
static constexpr std::array valid_colors{ "amb", "blu", "brn", "gry", "grn", "hzl", "oth" };

static const std::array<std::function<bool(std::string_view)>, tags.size()> validators{
  /*byr*/ [](std::string_view data) { return is_number_between(data, std::pair{ 1920, 2002 }); },
  /*iyr*/ [](std::string_view data) { return is_number_between(data, std::pair{ 2010, 2020 }); },
  /*eyr*/ [](std::string_view data) { return is_number_between(data, std::pair{ 2020, 2030 }); },
  /*hgt*/ [](std::string_view data) {
        if (const auto measurement = data.substr(0, data.size() - 2); data.ends_with("cm")) {
            return is_number_between(measurement, std::pair{ 150, 193 });
        } else if (data.ends_with("in")) {
            return is_number_between(measurement, std::pair{ 59, 76 });
        } else {
          return false;
        } },
  /*hcl*/ [](std::string_view data) { return is_valid_hex(data); },
  /*ecl*/ [](std::string_view data) { return ranges::contains(valid_colors, data); },
  /*pid*/ [](std::string_view data) { return data.size() == 9 && ranges::all_of(data, [](auto c) { return c >= '0' && c <= '9'; }); },
  /*cid*/ [](std::string_view data) { return true; },
};

std::string_view tag(std::string_view field)
{
  return field.substr(0, 3);
}

std::string_view value(std::string_view field)
{
  return field.substr(4);
}

using passport_t = std::array<std::string, tags.size()>;

std::vector<passport_t> parse(std::istream &&is)
{
  std::vector<passport_t> result;

  std::string current_field;
  passport_t current_passport;
  bool has_only_valid_tags = true;
  while (!is.eof()) {
    is >> current_field;
    const auto id_it = ranges::find(tags, tag(current_field));
    if (id_it == tags.cend()) {
      has_only_valid_tags = false;
    } else if (const auto index = std::distance(tags.cbegin(), id_it); current_passport[index].empty()) {
      current_passport[index] = value(current_field);
    } else {
      has_only_valid_tags = false;
    }

    is.ignore();
    if (is.peek() == '\n') {
      is.ignore();
      if (has_only_valid_tags) {
        result.push_back(current_passport);
      }
      current_passport.fill("");
      has_only_valid_tags = true;
    }
  }
  result.push_back(current_passport);

  return result;
}

bool is_valid_1(const passport_t &data)
{
  return ranges::none_of(data | ranges::views::drop_last(1), &std::string::empty);
}

bool is_valid_2(const passport_t &data)
{
  return ranges::all_of(
    ranges::views::zip(validators, data),
    [](const auto &el) { return el.first(el.second); });
}


int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", ranges::count_if(data, is_valid_1));
  fmt::print("Part 2: {}\n", ranges::count_if(data, is_valid_2));
}
