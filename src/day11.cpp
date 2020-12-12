#include "input_file_loader.h"

#include <range/v3/all.hpp>
#include <fmt/core.h>
#include <utility>

template <typename T>
class PaddedVector2D
{
public:
  PaddedVector2D(std::size_t n_cols, T m_padding_value, std::vector<T> raw)
    : m_raw{ std::move(raw) }
    , m_n_cols{ n_cols }
    , m_padding_value{ std::move(m_padding_value) }
  {}

  const T& at(int row, int column) const
  {
    if (row < 0 || row > rows() - 1 || column < 0 || column > cols() - 1) {
      return m_padding_value;
    } else {
      return m_raw[index(row, column)];
    }
  }

  T& at(int row, int column)
  {
    if (row < 0 || row > rows() - 1 || column < 0 || column > cols() - 1) {
      return m_padding_value;
    } else {
      return m_raw[index(row, column)];
    }
  }

  std::size_t rows() const
  {
    return m_raw.size() / m_n_cols;
  }

  std::size_t cols() const
  {
    return m_n_cols;
  }

  const std::vector<T>& raw() const
  {
    return m_raw;
  }

private:
  std::size_t index(int row, int column) const
  {
    return m_n_cols * row + column;
  };

  std::vector<T> m_raw;
  std::size_t m_n_cols;
  T m_padding_value;
};

enum class SeatState {
  Empty,
  Occupied,
  Missing
};

PaddedVector2D<SeatState> parse(std::istream &&is) {
  std::vector<SeatState> result;
  std::size_t column_size = 0;
  for (std::string line; std::getline(is, line);) {
    if (column_size == 0) column_size = line.size();
    ranges::transform(
      line,
      std::back_inserter(result),
      [](char c) { return c == 'L' ? SeatState::Empty : SeatState::Missing; });
  }
  
  return PaddedVector2D{ column_size, SeatState::Empty, std::move(result) };
}

constexpr std::array neighbours {
  std::pair{-1, -1},
  std::pair{-1, 0},
  std::pair{-1, 1},
  std::pair{0, -1},
  std::pair{0, 1},
  std::pair{1, -1},
  std::pair{1, 0},
  std::pair{1, 1},
};

constexpr auto count_neighbours_1 = [](const PaddedVector2D<SeatState>& data, int row, int column)
{
  return ranges::count(
    neighbours
      | ranges::views::transform([&data, row, column](auto p) { return data.at(p.first + row, p.second + column); }),
    SeatState::Occupied);
};

constexpr auto next_state_1 = [](SeatState old, int neighbours) {
  if (old == SeatState::Occupied && neighbours >= 4)
    return SeatState::Empty;
  else if (old == SeatState::Empty && neighbours == 0)
    return SeatState::Occupied;
  else
    return old;
};

constexpr auto count_neighbours_2 = [](const PaddedVector2D<SeatState>& data, int row, int column)
{
  return ranges::count(
    neighbours
      | ranges::views::transform([&data, row, column](auto p) { 
          auto r = p.first + row;
          auto c = p.second + column;
          while (data.at(r, c) == SeatState::Missing) {
            r += p.first;
            c += p.second;
          }
          return data.at(r, c);
      }),
    SeatState::Occupied);
};

constexpr auto next_state_2 = [](SeatState old, int neighbours) {
  if (old == SeatState::Occupied && neighbours >= 5)
    return SeatState::Empty;
  else if (old == SeatState::Empty && neighbours == 0)
    return SeatState::Occupied;
  else
    return old;
};

template <auto next_state, auto count_neighbours>
PaddedVector2D<SeatState> simulate_until_steady(PaddedVector2D<SeatState> arrangement)
{
  int diff = 0;
  auto old = arrangement;
  do {
    diff = 0;
    for (int row = 0; row < arrangement.rows(); ++row) {
      for (int column = 0; column < arrangement.cols(); ++column) {
        const auto old_value = old.at(row, column);
        const auto new_value = next_state(old_value, count_neighbours(old, row, column));
        if (old_value != new_value) { ++diff; }
        arrangement.at(row, column) = new_value;
      }
    }
    std::swap(arrangement, old);
  } while (diff > 0);
  return old;
}

int main(int argc, char **argv)
{
  const auto data = parse(load_input(argc, argv));
  fmt::print("Part 1: {}\n", ranges::count(simulate_until_steady<next_state_1, count_neighbours_1>(data).raw(), SeatState::Occupied));
  fmt::print("Part 2: {}\n", ranges::count(simulate_until_steady<next_state_2, count_neighbours_2>(data).raw(), SeatState::Occupied));
}