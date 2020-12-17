#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <utility>
#include <charconv>
#include <regex>

struct Field
{
  std::string name;
  std::pair<int, int> interval1;
  std::pair<int, int> interval2;
};

bool verify(std::pair<int, int> interval, int value) {
  return std::clamp(value, interval.first, interval.second) == value;
}

bool verify(const Field& field, int value) {
  return verify(field.interval1, value) || verify(field.interval2, value);
}

static const std::regex field_regex { R"(([^:]+): (\d+)-(\d+) or (\d+)-(\d+))" };

using ticket_t = std::vector<int>;

int to_int(std::string_view sv) {
  int i;
  std::from_chars(sv.data(), sv.data() + sv.size(), i);
  return i;
}

ticket_t parse_ticket(const std::string& ticket) {
  ticket_t result;
  auto begin = ticket.cbegin();
  for (;;) {
    const auto end = std::find(begin + 1, ticket.cend(), ',');
    result.push_back(to_int(std::string_view(begin, end)));
    if (end != ticket.cend()) {
      begin = end + 1;
    } else {
      break;
    }
  };
  return result;
}

std::tuple<std::vector<Field>, ticket_t, std::vector<ticket_t>> parse(std::istream &&is)
{
  std::vector<Field> fields;
  ticket_t my_ticket;
  std::vector<ticket_t> tickets;
  for (std::string line; std::getline(is, line) && !line.empty();) {
    std::smatch match;
    if (std::regex_match(line, match, field_regex)) {
      fields.emplace_back(
          match[1],
          std::pair{ std::stoi(match[2]), std::stoi(match[3]) },
          std::pair{ std::stoi(match[4]), std::stoi(match[5]) });
    }
  }
  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  {
    std::string line;
    std::getline(is, line);
    my_ticket = parse_ticket(line);
  }
  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  is.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  for (std::string line; std::getline(is, line) && !line.empty();) {
    tickets.push_back(parse_ticket(line));
  }
  return std::tuple{ std::move(fields), std::move(my_ticket), std::move(tickets) };
}

bool is_valid(const std::vector<Field>& fields, int value) {
  return ranges::any_of(fields, [value](const auto &f) { return verify(f, value); });
}

int ticket_scanning_error(const std::vector<Field> &fields, const std::vector<ticket_t> &values)
{
  return ranges::accumulate(
    values
      | ranges::views::join
      | ranges::views::filter(
        [&](int i) {
          return !is_valid(fields, i);
        }),
    0);
}

bool is_valid(const std::vector<Field>& fields, const ticket_t& t) {
  return ranges::all_of(t, [&](auto v) { return is_valid(fields, v); });
}

std::vector<std::vector<std::int8_t>> extract_possible(const std::vector<Field>& fields, const std::vector<ticket_t>& tickets)
{
  std::vector possible(fields.size(), std::vector(fields.size(), std::int8_t{ 1 }));
  for (const auto &t : tickets) {
    for (int i = 0; i < t.size(); ++i) {
      for (int j = 0; j < t.size(); ++j) {
        if (possible[i][j] != 0 && !verify(fields[j], t[i])) possible[i][j] = 0;
      }
    }
  }
  return possible;
}

std::vector<std::int8_t> extract_column(std::vector<std::vector<std::int8_t>> col_masks) {
  std::vector result(col_masks.size(), std::int8_t{ -1 });
  auto it = ranges::find_if(col_masks, [](const auto &c) { return ranges::count(c, std::int8_t{ 1 }) == 1; });
  while (it != col_masks.end()) {
    const auto col_ind = std::distance(col_masks.begin(), it);
    const auto col_mapped = std::distance(col_masks[col_ind].begin(), ranges::find(col_masks[col_ind], 1));
    result[col_mapped] = static_cast<int>(col_ind);
    ranges::for_each(col_masks, [col_mapped](auto &&m) { m[col_mapped] = 0; });
    it = ranges::find_if(col_masks, [](const auto &c) { return ranges::count(c, std::int8_t{ 1 }) == 1; });
  }
  return result;
}

int main(int argc, char **argv)
{
  const auto [fields, my_ticket, values] = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", ticket_scanning_error(fields, values));

  auto tickets = values
                 | ranges::views::filter([&](const auto &vs) { return is_valid(fields, vs); })
                 | ranges::to<std::vector>;
  const auto columns = extract_column(extract_possible(fields, tickets));
  fmt::print(
    "Part 2: {}\n",
    ranges::accumulate(
      ranges::views::zip(fields | ranges::views::transform(&Field::name), columns)
        | ranges::views::filter([](const auto &el) { return el.first.starts_with("departure"); })
        | ranges::views::transform([&](const auto &el) { return my_ticket[el.second]; }),
      1ll,
      std::multiplies{}
    )
  );
}
